
/*
 * Originally 2009 Robert Nagy <robert@openbsd.org>
 * See copyright notice at the original.
 */

/* System includes. */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <login_cap.h>
#include <bsd_auth.h>
#include <pwd.h>
#include <err.h>
#include <util.h>
#include <readpassphrase.h>
#include<sys/stat.h>
#include<dirent.h>

/* Fingerprint library. */
#include <fprint.h>

/* Our header. */
#include "common.h"

/* Truth. */
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* End of string character. */
#define EOS '\0'

/* Status code returned. */
#define SUCCESS 0

/* Authentication result. */
#define AUTH_OK 0
#define AUTH_FAILED 1

/* Fingerprint reader not found. */
#define NODEVICE -1

/* Maximum number of swipes. */
#define MAX_SWIPES 3

/* Response and passphrase size. */
#define MAXPASS 255

/* Capability string in login.conf. Finger id. */
#define CAP_FINGERPRINT	"x-fingerprint"

/* Globals. */

/* Debug flag. */
static int debug = false;

/* Verbose messages. */
static int verbose = true;

/* Back channel. Handle 3. */
static FILE *back = NULL;

/* Fingerprint data. */
static struct fp_dscv_dev *ddev = NULL;
static struct fp_dscv_dev **discovered_devs = NULL;
static struct fp_dev *dev = NULL;
static struct fp_print_data *data = NULL;

/* Password. */
static char passphrase[MAXPASS + 1];
static char response[MAXPASS + 1];

/* Cleanup. */

static void
cleanup (void)
{
	(void) memset (passphrase, 0, sizeof (passphrase));
	(void) memset (response, 0, sizeof (response));
	if (data != NULL)
	{
		fp_print_data_free (data);
	}
	if (dev != NULL)
	{
		fp_dev_close (dev);
	}
	if (discovered_devs != NULL)
	{
		fp_dscv_devs_free (discovered_devs);
	}
	fp_exit ();
	closelog ();
	(void) fclose (back);
}

/* Message with new line. */

static void
msg (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vprintf (format, args);
	(void) printf ("\n");
	(void) fflush (stdout);
	va_end (args);
}

/* Message without new line. */

static void
msgn (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vprintf (format, args);
	(void) fflush (stdout);
	va_end (args);
}

/* Log message in syslog. */

static void
log (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	if (debug)
	{
		(void) vprintf (format, args);
		(void) printf ("\n");
	}
	else
	{
		(void) vsyslog (LOG_INFO, format, args);
	}
	va_end (args);
}

/* Error message in syslog. */

static void
logerr (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	if (debug)
	{
		(void) vfprintf (stderr, format, args);
		(void) fprintf (stderr, "\n");
	}
	else
	{
		(void) vsyslog (LOG_ERR, format, args);
	}
	va_end (args);
}

/* Error exit. */

static void
error (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	if (debug)
	{
		(void) vfprintf (stderr, format, args);
		(void) fprintf (stderr, "\n");
	}
	else
	{
		(void) vsyslog (LOG_ERR, format, args);
	}
	va_end (args);

	/* Quit. */
	cleanup ();
	exit (AUTH_FAILED);
}

/* Read one character on the back channel. */

static char
backread (void)
{
	ssize_t nbytes;
	char ch;

	nbytes = read (3, &ch, (size_t) 1);
	if (nbytes < 0)
	{
		error ("Error %d reading back channel", errno);
	}
	else if (nbytes > (ssize_t) 1)
	{
		error ("Wrong size when read on back channel - confused");
	}
	return (ch);
}

/* Read response on back channel and copy password. */

static void
readresponse (char *r, size_t sr, char *p, size_t sp)
{
	int neos;
	char *rp;
	char *pp;
	int nch;
	int npass;
	char ch;
	int firsteos;

	neos = 0;
	rp = r;
	pp = p;
	nch = 0;
	npass = 0;
	while (neos < 2)
	{
		ch = backread ();
		if (ch == EOS)
		{
			firsteos = (neos == 0);
			neos++;
		}
		else
		{
			firsteos = false;
		}
		*rp = ch;
		rp++;
		nch++;
		if (nch >= sr)
		{
			error ("Response too long");
		}
		if (neos > 0)
		{
			if (! firsteos)
			{
				*pp = ch;
				pp++;
				npass++;
				if (npass >= sp)
				{
					error ("Passphrase too long");
				}
			}
		}
	}
}

/* Authentication failed, with message. */

static void
fail (char *msg)
{

	/* Looks like we need to flush. */
	if (msg != NULL)
	{
		if (*msg != EOS)
		{
			(void) fprintf (back, BI_VALUE " errormsg %s\n", msg);
		}
	}
	(void) fprintf (back, BI_REJECT "\n");
	(void) fflush (back);
	cleanup ();
	exit (AUTH_FAILED);
}

/* Authentication did pass. */

static void
pass (void)
{
	(void) fprintf (back, BI_AUTH "\n");
	(void) fflush (back);
	cleanup ();
	exit (AUTH_OK);
}

/* Perform passwd authentication style check. */

static void
passwdcheck (char *prompt, char *username)
{
	int status;
	char *rpass;

	/* Read the password. Use global.
	   When prompt is NULL it is interpreted that we already
	   got the password in the global 'passphrase'. */
	if (prompt != NULL)
	{
		rpass = readpassphrase (prompt, passphrase, sizeof (passphrase),
			RPP_ECHO_OFF);
		if (rpass == NULL)
		{
			logerr ("Error %d reading password for %s", errno, username);
			fail ("Error reading password");
		}
	}

	/* Password is in a global. */
	status = auth_userokay (username, "passwd", NULL, passphrase);
	if (status == 0)
	{

		/* Fail, bail out. */
		log ("Password authentication failure for %s", username);
		fail ("Password authentication failed");
	}
	else
	{

		/* Success, exit here. */
		log ("Password authentication success for %s", username);
		pass ();
	}
}

/* Discover device. Assumed that we got one USB reader. */

struct fp_dscv_dev *
discover (struct fp_dscv_dev **discovered_devs)
{
	struct fp_dscv_dev *ddev = NULL;
	struct fp_driver *drv = NULL;

	ddev = discovered_devs[0];
	if (ddev == NULL)
	{

		/* An empty list was returned - confused. */
		return (NULL);
	}

	/* Get the driver, just extra. */
	drv = fp_dscv_dev_get_driver (ddev);
	if (verbose)
	{
		msg ("Device %s (%s)", fp_driver_get_name (drv),
			fp_driver_get_full_name (drv));
	}
	return (ddev);
}

/* Discover and open device. */

static int
initialize (void)
{
	int status;

	/* Initialize library. */
	status = fp_init ();
	if (status < 0)
	{
		error ("Failed to initialize");
	}

	/* Check if the fingerprint reader is present. Uses global variables.
	   We will recourse to password authentication when not present. */
	discovered_devs = fp_discover_devs ();
	if (discovered_devs == NULL)
	{
		logerr ("Failed to discover fingerprint device");
		return (NODEVICE);
	}
	ddev = discover (discovered_devs);
	if (ddev == NULL)
	{
		logerr ("Unable to find a fingerprint reader");
		return (NODEVICE);
	}

	/* Open device. */
	dev = fp_dev_open (ddev);
	if (dev == NULL)
	{
		error ("Unable to open device");
	}
	return (SUCCESS);
}

/* Get stored fingerprint. */

static void
getprint (char *username, int fnum)
{
	int status;
	struct passwd *pe;
	char *home;

	if (fnum < FINGER_MIN || fnum > FINGER_MAX)
	{
		error ("Wrong finger number");
	}
	if (verbose)
	{
		log ("Read saved fingerprint");
	}

	/* Get home directory. */
	pe = getpwnam (username);
	if (pe == NULL)
	{
		error ("Unknown user %s", username);
	}

	/* Appearantly fp_print_data_load finds the prints from under
	   $HOME. So we need a bit of massaging here.
	   We load fingerprint into globals. */
	home = getenv ("HOME");
	status = setenv ("HOME", pe->pw_dir, 1);
	if (status < 0)
	{
		error ("Error setting home");
	}
	status = fp_print_data_load (dev, fnum, &data);
	if (status != 0)
	{
		error ("Failed to load fingerprint");
	}
	if (home != NULL)
	{
		status = setenv ("HOME", home, 1);
		if (status < 0)
		{
			error ("Error restoring home");
		}
	}
	else
	{
		status = unsetenv("HOME");
		if (status < 0)
		{
			error ("Function unsetenv returned an error");
		}
	}
}

/* Verify fingerprint, swiping. */

static int
verifyswipe (struct fp_dev *dev, struct fp_print_data *data, int fnum)
{
	int i;
	int status;

	for (i=1; i<=MAX_SWIPES; i++)
	{
		msgn ("Swipe your %s finger: ", fingernames[fnum]);

		/* Verify. */
		status = fp_verify_finger(dev, data);
		if (status < 0)
		{

			/* Unrecoverable error. */
			logerr ("Error %d scanning finger - verification failed", status);
			return (AUTH_FAILED);
		}
		switch (status)
		{
		case FP_VERIFY_NO_MATCH:

			/* No match, quit here. */
			logerr ("Login incorrect");
			return (AUTH_FAILED);
		case FP_VERIFY_MATCH:

			/* We are fine. */
			msg ("Okay");
			return (AUTH_OK);
		case FP_VERIFY_RETRY:
			msg ("Scan did not work, please try again");
			break;
		case FP_VERIFY_RETRY_TOO_SHORT:
			msg ("Swipe was too short, please try again");
			break;
		case FP_VERIFY_RETRY_CENTER_FINGER:
			msg ("Please center your finger on the sensor and try again");
			break;
		case FP_VERIFY_RETRY_REMOVE_FINGER:
			msg ("Please remove finger from the sensor and try again.");
			break;
		}
    }

	/* Three strikes, you are out. */
	logerr ("Fingerprint verification failed after %d tries with error %d", i, status);
	return (AUTH_FAILED);
}

/* Main. */

int
main(int argc, char **argv)
{
	int opt;
	char *optstring = "ds:v:";
	login_cap_t	*lc;
	int fnum = 0;
	int status;
	char *username = NULL;
	char *finger = NULL;
	char *class = NULL;
	char *service = NULL;

	/* Default. */
	setpriority (PRIO_PROCESS, 0, 0);

	/* Open syslog. */
	openlog (NULL, LOG_ODELAY, LOG_AUTH);

	/* Options. */
	opterr = 0;
	opt = getopt(argc, argv, optstring);
	while (opt != -1)
	{
		switch (opt)
		{
		case 'd':

			/* Debug. */
			debug = true;
			back = stdout;
			break;
		case 's':

			/* Service. */
			service = optarg;
			if (strcmp(service, "login") != 0 &&
			    strcmp(service, "challenge") != 0 &&
			    strcmp(service, "response") != 0)
			{
				error ("Service %s not supported", service);
			}
            break;
		case 'v':

			/* Ignore variables. */
			if (debug)
			{
				log ("Variable %s", optarg);
			}
			break;
		default:
			error ("Wrong switch");
			break;
		}
		opt = getopt(argc, argv, optstring);
	}

	/* Get username and class. */
	switch (argc - optind)
	{
	case 2:
		class = argv[optind + 1];
		/* Fallthrough! */
	case 1:
		username = argv[optind];
		break;
	default:
		error ("Usage error");
		break;
	}

	/* If not debug open it. */
	if (back == NULL)
	{
		back = fdopen (3, "r+");
		if (back == NULL)
		{
			error ("Error reopening back channel");
		}
	}

	/* Get the login class. */
	lc = login_getclass(class);
	if (lc == NULL)
	{
		logerr ("Cannot find class %s in login.conf", class);
		error ("Cannot find login class");
	}

	/* Finger number from login.conf. */
	finger = login_getcapstr(lc, CAP_FINGERPRINT, NULL, NULL);
	if (finger == NULL)
	{
		logerr ("You need to set %s for %s", CAP_FINGERPRINT, lc->lc_class);
		error ("Fingerprint number not set in config file");
	}

	/* Response protocol as per the book. */
	if (strcmp(service, "challenge") == 0)
	{
		(void) fprintf (back, BI_SILENT "\n");
		exit (0);
	}

	/* Get the password if response. */
	if (strcmp (service, "response") == 0)
	{

		/* Try to play response protocol. */
		(void) memset (response, 0, sizeof (response));
		(void) memset (passphrase, 0, sizeof (passphrase));
		readresponse (response, sizeof (response),
			passphrase, sizeof (passphrase));
		if (*response != EOS)
		{
			error ("Response protocol error");
		}
		if (verbose)
		{
			log ("Having a response");
		}
	}

	/* Check if the fingerprint reader is present. Initialization
	   will bail out when there is an error and returns with NODEVICE
	   when no fingerprint reader present. Recourse to password
	   authentication if no device detected. */
	status = initialize ();
	if (status == NODEVICE)
	{

		/* No fingerprint reader. */
		if (verbose)
		{
			log ("No fingerprint reader");
		}
		if (strcmp (service, "response") == 0)
		{
			if (*passphrase != EOS)
			{

				/* In case this is an xdm login we have the password in the response.
				   Otherwise, as a console login via login_seq we are also in response
				   mode but the password is the empty string. */
				passwdcheck (NULL, username);
			}
			else
			{
				passwdcheck ("No fingerprint reader\nEnter password: ", username);
			}
		}
		else
		{

			/* Running login_fingerprint as native. */
			passwdcheck ("Cannot find fingerprint device\nEnter password: ", username);
		}
	}

	/* Finger number. */
	fnum = atoi (finger);
	if (fnum == 0)
	{
		error ("Wrong finger %s", finger);
	}
	if (fnum < FINGER_MIN)
	{
		error ("Finger number too large");
	}
	if (fnum > FINGER_MAX)
	{
		error ("Finger number too small");
	}

	/* Get stored fingerprint. */
	getprint (username, fnum);

	/* Verify with swipe. */
	if (verbose)
	{
		log ("Verifying fingerprint %s", fingernames[fnum]);
	}
	status = verifyswipe (dev, data, fnum);
	if (verbose)
	{
		log ("Verifying fingerprint returned %d", status);
	}
	if (status == AUTH_OK)
	{
		log ("Authorized %s", username);
		pass ();
	}
	else
	{
		log ("Failure %s", username);
		fail ("Authentication failed");
	}

	/* Finish. Should never get here. */
	cleanup ();
	exit(AUTH_FAILED);
}

