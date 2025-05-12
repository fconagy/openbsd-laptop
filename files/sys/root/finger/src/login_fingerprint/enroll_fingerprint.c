
/* Enroll fingerprint, originally by Robert Nagy. */

/* Includes. */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <pwd.h>
#include <err.h>
#include <util.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <login_cap.h>
#include <bsd_auth.h>
#include <fprint.h>

/* Our include. */
#include "common.h"

/* Boolean. */
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* Status. */
#define SUCCESS 0
#define FAILURE 1

/* End of string character. */
#define EOS '\0'

/* Global fingerprint info. */
static struct fp_dscv_dev *ddev = NULL;
static struct fp_dscv_dev **discovered_devs = NULL;
static struct fp_dev *dev = NULL;
static struct fp_print_data *data = NULL;

/* Cleanup. */

static void
cleanup (void)
{
	if (data != NULL)
	{
		fp_print_data_free (data);
	}
	if (discovered_devs != NULL)
	{
		fp_dscv_devs_free(discovered_devs);
	}
	if (dev != NULL)
	{
		fp_dev_close (dev);
	}
	fp_exit ();
}

/* Error exit. */

static void
error (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vprintf (format, args);
	(void) printf ("\n");
	va_end (args);

	/* Quit. */
	cleanup ();
	exit (FAILURE);
}

static struct fp_dscv_dev *
discover_device (struct fp_dscv_dev **discovered_devs)
{
	struct fp_dscv_dev *ddev;

	ddev = discovered_devs[0];
	if (ddev == NULL)
	{
		return (NULL);
	}
	/* To get the driver. We don't need that here.
	struct fp_driver *drv;
	drv = fp_dscv_dev_get_driver(ddev);
	*/
	return (ddev);
}

/* Get fingerprint data. */

static struct fp_print_data *
enroll (struct fp_dev *dev, int finger)
{
	int status;
	int i = 1;
	int n;
	struct fp_print_data *enrolled_print = NULL;

	do
	{
		n = fp_dev_get_nr_enroll_stages (dev);
		(void) printf ("Please swipe your %s finger %d times ",
			fingernames[finger], n);
		(void) fflush (stdout);

		status = fp_enroll_finger (dev, &enrolled_print);
		if (status < 0)
		{
			error ("Enroll error %d", status);
		}
		switch (status)
		{
		case FP_ENROLL_COMPLETE:
			printf ("complete\n");
			break;
		case FP_ENROLL_FAIL:
			printf ("Enroll failed\n");
			return (NULL);
		case FP_ENROLL_PASS:
			printf ("Enroll passed\n");
			break;
		case FP_ENROLL_RETRY:
			printf ("Invalid swipe, please try again\n");
			break;
		case FP_ENROLL_RETRY_TOO_SHORT:
			printf ("Short swipe, please try again\n");
			break;
		case FP_ENROLL_RETRY_CENTER_FINGER:
			printf ("Positioning error, please center your finger on the "
				"sensor and try again\n");
			break;
		case FP_ENROLL_RETRY_REMOVE_FINGER:
			printf ("Swipe failed, please remove your finger and then try "
				"again\n");
			break;
		}
		i++;
		if (i > n + 1)
		{
			error ("Enroll failed too many times");
		}
	} while (status != FP_ENROLL_COMPLETE);
	if (enrolled_print == NULL)
	{
		error ("Couldn't process fingerprint, please try again");
	}
	printf ("Enrollment for %s finger completed\n", fingernames[finger]);
	return (enrolled_print);
}

/* Function. */

static void
fix (char *path, uid_t uid, gid_t gid)
{
	int status;
	struct stat sb;
	uid_t u;
	gid_t g;
	mode_t m;
	mode_t mode;

	status = stat (path, &sb);
	if (status < 0)
	{
		error ("Error calling stat");
	}
	u = sb.st_uid;
	g = sb.st_gid;
	m = sb.st_mode;
	mode = (mode_t) S_IRUSR|S_IWUSR;
	if (u != uid || g != gid)
	{
		status = chown (path, uid, gid);
		if (status < 0)
		{
			error ("Fixing ownership failed for %s", path);
		}
	}
	if ((m & S_IRWXG) != (mode_t) 0 ||
		(m & S_IRWXO) != (mode_t) 0)
	{
		status = chmod (path, mode);
		if (status < 0)
		{
			error ("Fixing protections failed for %s", path);
		}
	}
}

/* Traverse directory. */

static void
traverse (char *name,
	void (*f) (char *, uid_t, gid_t),
	uid_t uid, gid_t gid)
{
	DIR *dirp;
	struct dirent *dirent;
	char d[MAXNAMLEN + 1];
	char *dir;
	static int recurse = false;

	if (! recurse)
	{
		f (name, uid, gid);
	}
	dirp = opendir (name);
	dirent = readdir (dirp);
	while (dirent != NULL)
	{
		dir = dirent->d_name;
		if (! (strcmp (dir, ".") == 0 ||
			strcmp (dir, "..") == 0))
		{
			*d = '\0';
			(void) strncat (d, name, MAXNAMLEN);
			(void) strncat (d, "/", MAXNAMLEN);
			(void) strncat (d, dir, MAXNAMLEN);
			f (d, uid, gid);
			if (dirent->d_type == DT_DIR)
			{

				/* Directory. */
				recurse = true;
				traverse (d, f, uid, gid);
			}
			else if (dirent->d_type == DT_REG)
			{

				/* Regular file. */
				;
			}
		}
		dirent = readdir (dirp);
	}
	closedir(dirp);
}

/* Print help. */

static void
usage (void)
{
	(void) printf ("Usage: enroll_fingerprint -f num -u user\n");
	exit (FAILURE);
}

/* Main. */

int
main(int argc, char **argv)
{
	char *optstr = "hf:u:";
	int ch;
	int finger = -1;
	int status;
	char fprintdir[MAXNAMLEN + 1];
	char *username = NULL;
	struct passwd *pe;
	char *userhome = NULL;
	char *oldhome;
	uid_t uid;
	gid_t gid;

	/* Options. */
	opterr = 0;
	ch = getopt (argc, argv, optstr);
	while (ch != -1)
	{
		switch (ch)
		{
		case 'h':
			usage ();
			break;
		case 'f':
			finger = atoi (optarg);
			if (finger == 0)
			{
				error ("Wrong finger number %s", optarg);
			}
			if (finger > FINGER_MAX)
			{
				error ("Finger number %d too large", finger);
			}
			if (finger < FINGER_MIN)
			{
				error ("Finger number %d  too small", finger);
			}
			break;
		case 'u':
			username = optarg;
			if (username == NULL)
			{
				error ("You have to specify username with -u");
			}
			break;
		default:
			usage();
			break;
		}
		ch = getopt (argc, argv, optstr);
	}

	/* Check. */
	if (finger < 0)
	{
		usage ();
	}

	/* Initialize and discover device. */
	status = fp_init ();
	if (status < 0)
	{
		error ("Failed to initialize, status %d", status);
	}
	discovered_devs = fp_discover_devs ();
	if (discovered_devs == NULL)
	{
		error ("Could not discover fingerprint reader devices");
	}
	ddev = discover_device (discovered_devs);
	if (ddev == NULL)
	{
		error ("No fingerprint reader devices detected");
	}

	/* Open device. */
	dev = fp_dev_open(ddev);
	if (dev == NULL)
	{
		error ("Could not open device");
	}

	/* Enroll. */
	data = enroll (dev, finger);
	if (data == NULL)
	{
		error ("Could not get fingerprint");
	}

	/* Get user home directory and build fingerprint data
	   directory name. */
	pe = getpwnam (username);
	if (pe == NULL)
	{
		error ("User %s not found", username);
	}
	userhome = pe->pw_dir;
	uid = pe->pw_uid;
	gid = pe->pw_gid;

	/* Save data under $HOME/.fprint. */
	oldhome = getenv ("HOME");
	if (oldhome == NULL)
	{
		error ("No HOME");
	}
	status = setenv ("HOME", userhome, 1);
	if (status < 0)
	{
		error ("Error setting home");
	}
	status = fp_print_data_save (data, finger);
	if (status < 0)
	{
		error ("Data save failed with error code %d", status);
	}
	status = setenv ("HOME", oldhome, 1);
	if (status < 0)
	{
		error ("Cannot restore home");
	}

	/* Fix ownership and protections. */
	*fprintdir = EOS;
	strncat (fprintdir, userhome, MAXNAMLEN);
	strncat (fprintdir, "/.fprint", MAXNAMLEN);
	traverse (fprintdir, fix, uid, gid);

	/* Finish. */
	cleanup ();
	exit (SUCCESS);
}

