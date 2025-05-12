
/* LOGIN_SEQ.C implements a sequence of login styles. */

/* Includes. */
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <stdarg.h>
#include <pwd.h>
#include <util.h>
#include <login_cap.h>
#include <readpassphrase.h>
#include <bsd_auth.h>

/* Status codes. */
#define AUTHPASS 0
#define AUTHFAIL 1

/* Truth. */
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* Upper limit of strings. */
#define LONGSTRINGLENGTH 1024

/* End of string character. */
#define EOS '\0'

/* Delimiter between termcap string authentication style items. */
#define COMMA ','

/* Don't pledge.
#define DONTPLEDGE 1
*/

/* Global variables. */

/* Debug switch. */
static int debug = false;

/* Verbose logging. */
static int verbose = true;

/* Enable extra test options. */
static int extras = true;

/* Syslog name/ident. */
static char *ident = "seq";

/* Back channel file. */
static FILE *back;

/* Variables array. */
#define MAXVAR 255
static int nvar = 0;
static char *variables[MAXVAR];

/* Number of items in a capability list. */
#define MAXCAP 255

/* Authentication styles array. */
static int nstyle = 0;
static char *styles[MAXCAP];
static char *capstyles = NULL;

/* Prompts array. */
static int nprompt = 0;
static char *prompts[MAXCAP];
static char *capprompts = NULL;

/* Passphrase. */
#define MAXPASSPHRASE 1024
static char passphrase[MAXPASSPHRASE + 1];

/* Challenge / response. */
#define MAXRESPONSE MAXPASSPHRASE
static char response[MAXRESPONSE + 1];

/* Cleanup. */

static void
cleanup (void)
{
	int i;

	(void) memset (passphrase, 0, sizeof (passphrase));
	(void) memset (response, 0, sizeof (response));
	for (i=0; i<nvar; i++)
	{
		free (variables[i]);
	}
	if (capstyles != NULL)
	{
		free (capstyles);
	}
	if (capprompts != NULL)
	{
		free (capprompts);
	}
	closelog ();
	(void) close (3);
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
		(void) vprintf (format, args);
		(void) printf ("\n");
	}
	else
	{
		(void) vsyslog (LOG_ERR, format, args);
	}
	va_end (args);

	/* Quit. */
	cleanup ();
	exit (AUTHFAIL);
}

/* Log message. */

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

/* Dynamic copy of a string. */

static char *
copy (char *s)
{
	char *r;

	r = strdup (s);
	if (r == NULL)
	{
		error ("String copy failed");
	}
	return (r);
}

/* String equal. */

static int
same (char *s1, char *s2)
{
	return (strcmp (s1, s2) == 0);
}

/* Issue pledge. */

static void
dopledge (char *promise)
{
#ifndef DONTPLEDGE
	int status;

	status = pledge (promise, NULL);
	if (status < 0)
	{
		if (debug)
		{
			error ("Calling pledge failed with %d (%s)", errno, promise);
		}
		else
		{
			error ("pledge: %m (%s)", promise);
		}
	}
#else
	;
#endif
}

/* New line on back channel. */

static void
nl (void)
{
	(void) fprintf (back, BI_SILENT "\n");
	(void) fflush (back);
}

/* Authentication failed. */

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
	exit (AUTHFAIL);
}

/* Authentication did pass. */

static void
pass (void)
{
	(void) fprintf (back, BI_AUTH "\n");
	(void) fflush (back);
	cleanup ();
	exit (AUTHPASS);
}

/* We have no use of default or error other than
   to detect the error condition. */
#define CAPERROR "error"

/* Get string capability. */

static char *
getcap (char *class, char *cap)
{
	login_cap_t *lc;
	char *r;

	lc = login_getclass (class);
	if (lc == NULL)
	{
		login_close (lc);
		error ("No class");
	}
	r = login_getcapstr (lc, cap, CAPERROR, CAPERROR);
	if (same (r, CAPERROR))
	{
		login_close (lc);
		error ("No cap %s", cap);
	}
	login_close (lc);
	return (copy (r));
}

/* Make array of strings out of the cap. */

static int
split (char *cap, int *n, char *a[], int max)
{
	char *cp;

	cp = cap;
	a[*n] = cp;
	(*n)++;
	while (*cp != EOS)
	{
		if (*cp == COMMA)
		{
			*cp = EOS;
			cp++;
			a[*n] = cp;
			(*n)++;
			if ((*n) >= max)
			{
				error ("Too many elements");
			}
		}
		else
		{
			cp++;
		}
	}
	return (*n);
}

/* Accept CH when it is in S. */

static int
accept (char ch, char *s)
{
	int i;
	int len;
	int found;

	found = false;
	len = (int) strlen (s);
	for (i=-0; i<len; i++)
	{
		if (s[i] == ch)
		{
			found = true;
			break;
		}
	}
	return (found);
}

/* Check string for strange characters. */

static void
checkstring (char *s, char *exceptions)
{
	int i;
	int len;
	char *additionals;
	int ch;

	/* Bumpers. */
	if (s == NULL)
	{
		error ("String NULL");
	}
	len = (int) strlen (s);
	if (len >= LONGSTRINGLENGTH)
	{
		error ("String too long");
	}
	if (exceptions == NULL)
	{
		additionals = "-_.@";
	}
	else
	{
		additionals = exceptions;
	}
	for (i=0; i<len; i++)
	{
		ch = s[i];

		/* Numbers, uppercase letters, lowercase letters,
		   hyphen, undersoce, dot, at sign. Only 7-bit ASCII, EBCDIC is
		   not supported. ;) */
		if ((ch >= 0x30 && ch <= 0x39) ||
			(ch >= 0x41 && ch <= 0x5a) ||
			(ch >= 0x61 && ch <= 0x7a) ||
			accept (ch, additionals))
		{

			/* OK. */
			;
		}
		else
		{

			/* Bail out. */
			error ("Invalid character in string");
		}
	}
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

/* Get the passprase string from a saved response buffer. */

static void
resp2pass (char *response)
{
	char *p;
	char *r;
	char ch;

	p = passphrase;
	r = response;
	r++;
	ch = *r;
	while (ch != EOS)
	{
		*p = ch;
		r++;
		p++;
		ch = *r;
	}
	*p = EOS;
}

/* Main program. */

int
main (int argc, char *argv[], char *envp[])
{

	/* Option string and current option character. */
	char *options = "ds:v:t:";
	int optchar;

	/* Terminal fd. */
	int ttyfd;

	/* Service name. */
	char *service = NULL;

	/* Mode base on service. */
	int mode = 0;

	/* Test name. */
	char *test = NULL;

	/* Number of real arguments. */
	int narg;

	/* Class and username. */
	char *variable = NULL;
	char *username = NULL;
	char *class = NULL;

	/* Status code from library call. */
	int status;

	/* Counter. */
	int i;

	/* Environment variable. */
	char *e = NULL;

	/* Passphrase returned. */
	char *rpass = NULL;

	/* Password to use. */
	char *password = NULL;

	/* Style. */
	char *style = NULL;

	/* Prompt. */
	char *prompt = NULL;

	/* Authentication type passed to auth_userokay. */
	char *authtype = NULL;

	/* Flag to signal not to call readpassphrase. */
	int dontask = false;

	/* Same as above, explicitely asked with empty prompt. */
	int noprompt = false;

	/* Set up signal handling. */
	(void) signal (SIGQUIT, SIG_IGN);
	(void) signal (SIGINT, SIG_IGN);

	/* Set priority to default. */
	(void) setpriority (PRIO_PROCESS, 0, 0);

	/* Open syslog. The default name would be starting
	   with a dash for local style. */
	openlog (ident, LOG_ODELAY, LOG_AUTH);

	/* Deal with the options. */
	opterr = 0;
	optchar = getopt (argc, argv, options);
	while (optchar != -1)
	{
		switch (optchar) {
		case 'd':

			/* Debug. */
			debug = true;
			break;
		case 's':

			/* Service name. */
			service = optarg;
			if (service == NULL)
			{
				error ("No argument to service");
			}
			checkstring (service, NULL);
			if (same (service, "login"))
			{
				mode = 1;
			}
			else if (same (service, "challenge"))
			{
				mode = 2;
			}
			else if (same (service, "response"))
			{
				mode = 3;
			}
			else
			{
				error ("Wrong argument to service - %s", service);
			}
			break;
		case 't':

			/* Test. When no extras just ignore it. */
			if (extras)
			{
				test = optarg;
				if (test == NULL)
				{
					error ("No argument to test");
				}
			}
			break;
		case 'v':

			/* Variable. */
			if (nvar >= MAXVAR)
			{
				error ("Too many variables");
			}
			variable = optarg;
			if (variable == NULL)
			{
				error ("No argument to variable");
			}
			checkstring (variable, "_=");
			variables[nvar] = copy (variable);
			nvar++;
			break;
		default:
			error ("Usage error");
			break;
		}
		optchar = getopt (argc, argv, options);
	}

	/* Arguments. */
	narg = argc - optind;
	if (narg > 0)
	{
		username = argv[optind];
		checkstring (username, NULL);
	}
	if (narg > 1)
	{
		class = argv[optind + 1];
		checkstring (class, NULL);
	}
	if (narg > 2)
	{
		error ("Usage error");
	}

	/* Log what we got. */
	if (verbose)
	{
		if (service != NULL)
		{
			log ("Service is %s", service);
		}
		for (i=0; i<nvar; i++)
		{
			log ("Variable %d is %s", i, variables[i]);
		}
		if (username != NULL)
		{
			log ("Username is %s", username);
		}
		if (class != NULL)
		{
			log ("Class is %s", class);
		}
		if (mode != 0)
		{
			log ("Mode is %d", mode);
		}
		i = 0;
		e = envp[i];
		while (e != NULL)
		{
			log ("Environment[%d] is '%s'", i, e);
			i++;
			e = envp[i];
		}
	}

	/* Pledge. We will need getpw to be able to do passwd
	   authentication with auth_userokay. We need tty access
	   to read the passphrase. We cannot restrain any more
	   since when authenticating in a sequence we need tty
	   access again and abilities cannot be regained with
	   pledge. With pledge issues authentication just fails
	   silently. Appearantly we also need proc and exec. */
	dopledge ("stdio rpath tty id getpw proc exec");

	/* Initialization. */
	if (debug)
	{
		ttyfd = open ("/dev/tty", O_RDWR);
		if (ttyfd < 0)
		{
			error ("Error opening tty");
		}
		back = fdopen (ttyfd, "r+");
		if (back == NULL)
		{
			error ("reopening back channel: %m");
		}
	}
	else
	{

		/* Reopen back channel as fd 3. */
		back = fdopen (3, "r+");
		if (back == NULL)
		{
			error ("reopening back channel: %m");
		}
	}

	/* Play the protocol. */
	if (mode == 2)
	{

		/* Challenge. Return with 0 as in the response
		   section of 'man login.conf'. */
		if (verbose)
		{
			log ("Challenge - quit here");
		}
		(void) fprintf (back, BI_SILENT "\n");
		(void) fflush (back);
		exit (0);
	}
	if (mode == 3)
	{

		/* Get passphrase from response. */
		if (verbose)
		{
			log ("Read response");
		}
		(void) memset (passphrase, 0, sizeof (passphrase));
		readresponse (response, sizeof (response),
			passphrase, sizeof (passphrase));
		if (*response != EOS)
		{
			error ("Response protocol error");
		}
		dontask = true;
	}

	/* Get authentication style info from login.conf. */
	capstyles = getcap (class, "styles");
	split (capstyles, &nstyle, styles, MAXCAP);
	capprompts = getcap (class, "prompts");
	split (capprompts, &nprompt, prompts, MAXCAP);
	if (nstyle != nprompt)
	{
		error ("Authentication styles and prompts do not match");
	}

	/* Process authentication sequence. */
	for (i=0; i<nstyle; i++)
	{

		/* Initialize, from login.conf. */
		style = styles[i];
		prompt = prompts[i];
		noprompt = false;
		if (same (prompt, ""))
		{

			/* Prompt is the empty string. We don't ask for
			   passphrase, let auth_okay do it. */
			noprompt = true;
		}

		/* Get the password. It's already in passphrase when
		   in response mode. Otherwise we'll ask for it. In
		   some cases we leave it to auth_userokay. */
		password = passphrase;
		if (! dontask)
		{

			/* Get passphrase. */
			if (! noprompt)
			{
				if (verbose)
				{
					log ("Read passphrase");
				}
				(void) memset (passphrase, 0, sizeof (passphrase));
				rpass = readpassphrase (prompt, passphrase, sizeof (passphrase),
					RPP_ECHO_OFF);
				if (rpass == NULL)
				{
					log ("Error %d reading password for %s", errno, username);
					fail ("Error reading password");
				}
			}
		}
		else
		{

			/* Are we in response mode, then we should already have the
			   password. */
			if (mode == 3)
			{

				/* The previous auth_userokay zeroed out the password so we
				   retrieve it from the 'response' global to passphrase. */
				resp2pass (response);
			}
			else
			{

				/* No password but maybe it's OK? */
				password = NULL;
			}
		}

		/* Do auth. */
		if (verbose)
		{
			log ("Authenticating %s style %s prompt '%s' (item %d)",
				username, style, prompt, i);
		}
		authtype = NULL;
		status = auth_userokay (username, style, authtype, password);
		if (status == 0)
		{

			/* Means failure in this case. */
			log ("Failed %s style %s", username, style);
			fail ("Authentication failed");

			/* Should never get here but just to be sure. */
			exit (AUTHFAIL);
		}
	}

	/* If we got here that means pass. */
	log ("Authenticated %s", username);
	pass ();

	/* Finish. Should never reach. */
	cleanup ();
	exit (AUTHFAIL);
}

/* End of file LOGIN_SEQ.C. */

