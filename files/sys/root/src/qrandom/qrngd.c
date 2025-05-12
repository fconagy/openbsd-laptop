
/* File QRNGD.C is a is a simple daemon to feed the random pool from
   the Quantis Quantum hardware RNG. Uses the mixrng device. */

/* System includes. */
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

/* USB library include. (From /usr/local/include.) */
#include <libusb-1.0/libusb.h>

/* Our header from under the srouce tree. */
#include "/usr/src/sys/dev/mixrngio.h"

/* Our Quantis include. */
#include "qusb.h"

/* Boolean constants, in case we don't have. */
#ifndef true
#define true ((int) 1)
#endif
#ifndef false
#define false ((int) 0)
#endif

/* End of string. */
#define EOS '\0'

/* Maximum buffer size. */
#define MAX_BUFFER 100000

/* One megabyte. */
#define MIB 1048576

/* Maximum amout to mix into the random pool in one go. */
#define MAX_MIX (2 * MIB)

/* Default buffersize. */
#define DEFAULT_BUFFERSIZE 512

/* Pid file name. */
#define DEFAULT_PIDFILE "/var/run/qrngd.pid"

/* Poll wait time for the device to come back online. Seconds. */
#define DEFAULT_POLL_WAIT 1

/* PID string buffer length. */
#define PID_MAX 64

/* Log file name. */
#define DEFAULT_LOGFILE "/var/log/qrngd.log"

/* Device file for mixrng. */
#define MIXRNG_DEVICE "/dev/mixrng"

/* Debug level. Compiled in. Note that debug level
   greater than 16 will disable functionality.
   Set to zero to leave debug prints but run-time
   disabled them or undefine to fully disable it. */
#define QRNGD_DEBUG 0

/* Debug. */
#ifdef QRNGD_DEBUG
static int debug = QRNGD_DEBUG;
#else
static int debug = 0;
#endif

/* Globals. */

/* Verbose flag. */
static int verbose = false;

/* PID file. */
static char *pidfile = DEFAULT_PIDFILE;

/* Log file. */
static char *logfile = DEFAULT_LOGFILE;

/* Log file handle. */
int logfd;

/* Device file name for mixrng. Disable touching the
   pool for high debug levels. */
#if QRNGD_DEBUG > 16
static char *mixrng_name = "/dev/null";
#else
static char *mixrng_name = MIXRNG_DEVICE;
#endif

/* QRNG device descriptor. */
static qdev_t *q = NULL;

/* Device file handle. */
static int mfd;

/* Buffer size in bytes. */
static int buffersize = DEFAULT_BUFFERSIZE;

/* Buffer for the returned random bytes. */
static char *buffer = NULL;

/* Microsecond sleep. */

static void
msleep (int delay)
{
	if (delay >= 1000000)
	{
		(void) sleep ((unsigned int) (delay / 1000000));
	}
	else
	{
		(void) usleep ((useconds_t) delay);
	}
}

/* Cleanup. */

static void
cleanup (void)
{
	(void) close (mfd);
	if (buffer != 0)
	{
		(void) memset (buffer, 0, buffersize);
		free (buffer);
	}
	qclose (q);
	qdestroy (q);
}

/* Error exit. */

static void
error (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vfprintf (stderr, format, args);
	va_end (args);
	(void) fprintf (stderr, "\n");
	(void) fflush (stderr);

	/* Cleanup and quit. */
	cleanup ();
	exit (FAILURE);
}

/* Print message. */

static void
msg (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vfprintf (stdout, format, args);
	va_end (args);
	(void) fprintf (stdout, "\n");
}

/* Print debug message. */

static void
dmsg (const char *format, ...)
{
    va_list args;

	/* Print message. */
	if (debug > 0)
	{
		(void) fprintf (stdout, "==== ");
		va_start (args, format);
		(void) vfprintf (stdout, format, args);
		va_end (args);
		(void) fprintf (stdout, "\n");
	}
}

/* Print message when verbose is on. */

static void
vmsg (const char *format, ...)
{
    va_list args;

	if (verbose)
	{
		va_start (args, format);
		(void) vfprintf (stdout, format, args);
		va_end (args);
		(void) fprintf (stdout, "\n");
	}
}

/* Show progress. */

static void
progress (char *s)
{
	if (verbose)
	{
		(void) fprintf (stdout, "%s", s);
		(void) fflush (stdout);
	}
}

/* Debug printf. */

#ifdef QRNGD_DEBUG
#define dpf(m) \
	if (debug > 0) \
	{ \
		(void) printf ("==== %s\n", m); \
	}
#else
#define dpf(m) /* Nothing. */
#endif

/* Display bytes. */

static void
bufprint (char *buffer, int nbytes)
{
	int i;

	for (i=0; i<nbytes; i++)
	{

		/* Print one byte in hex format. */
		(void) printf ("%02x", (unsigned char) buffer[i]);
	}
}

/* Open mixrng device file. */

static int
mixrng_open (void)
{
	int fd;

	fd = open (mixrng_name, O_WRONLY);
	if (fd < 0)
	{
		error ("Cannot open mixrng device file");
	}
	return (fd);
}

/* Switch off debug for mixrng. */

static void
mixrng_nodebug (int fd)
{
	struct mixrng_params params;
	int status;

	(void) memset (&params, 0, sizeof (struct mixrng_params));
	params.code = 0;
	status = ioctl (fd, MIXRNGIO_DEBUG, &params);
	if (status < 0)
	{
		error ("Function ioctl failed with %d", errno);
	}
}

/* Write one record to mixrng. */

static void
mixrng_write (int fd, char *buffer, int buffersize)
{
	ssize_t status;

	status = write (fd, (void *) buffer, (size_t) buffersize);
	if (status != (ssize_t) buffersize)
	{
		error ("Error writing mixrng");
	}
}

/* Close mixrng device file. */

static void
mixrng_close (int fd)
{
	int status;

	status = close (fd);
	if (status != 0)
	{
		error ("Error closing mixrng device");
	}
}

/* Macro to establish signal handler. */

#define sig(s, h) \
	if (signal (s, h) == SIG_ERR) \
	{ \
		error ("Establishing signal handler for signal %d failed with %d", \
			s, errno); \
	}

/* Our default signal handler. */

static void
defhand (int signum)
{

	/* Just clean up for any signal. */
	dpf ("signal");
	cleanup ();
	(void) unlink (pidfile);
	vmsg ("Exiting");
	exit (SUCCESS);
}

/* Switch output to log file. */

static void
switchlog (void)
{
	int status;

	logfd = open (logfile, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP);
	if (logfd < 0)
	{
		error ("Error %d opening log file %s", errno, logfile);
	}
	status = dup2 (logfd, STDOUT_FILENO);
	if (status < 0)
	{
		error ("Error %d redirecting stdout to log file", errno);
	}
	status = dup2 (logfd, STDERR_FILENO);
	if (status < 0)
	{
		error ("Error %d redirecting stderr to log file", errno);
	}
	status = close (logfd);
	if (status < 0)
	{
		error ("Error %d closing logfd", errno);
	}
}

/* Check if PID file is there. */

static void
checkpid ()
{
	int status;

	/* Statbuf. */
	struct stat sb;

	/* Check if PID file already exists. If it does
	   either it is running or possibly left behind. */
	status = stat (pidfile, &sb);
	if (status < 0)
	{
		if (errno != ENOENT)
		{
			error ("Error %d checking PID file %s", errno, pidfile);
		}
	}
	else
	{
		error ("PID file %s already exists - daemon possibly running, kill daemon or remove file to continue",
			pidfile);
	}
}

/* Remember pid. */

static void
savepid (pid_t pid)
{

	/* PID file handle. */
	int pidfd;

	/* Buffer to hold PID string. */
	char pidbuf[PID_MAX];

	/* Status code. */
	int status;

	(void) memset (pidbuf, 0, PID_MAX);
	status = snprintf (pidbuf, (size_t) PID_MAX, "%d", (int) pid);
	if ((status >= PID_MAX) || (status <= 0))
	{
		error ("Error converting PID");
	}
	pidfd = open (pidfile, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
	if (pidfd < 0)
	{
		error ("Could not create PID file, error code %d", errno);
	}
	status = write (pidfd, pidbuf, strlen (pidbuf));
	if (status < 0)
	{
		error ("Error %d writing PID file", errno);
	}
	status = close (pidfd);
	if (status < 0)
	{
		error ("Error %d closing PID file", errno);
	}
}

/* Get the pid from the saved PID file. */

static pid_t
daemonpid ()
{

	/* PID file handle. */
	int pidfd;

	/* Buffer to hold PID string. */
	char pidbuf[PID_MAX];

	/* Daemon PID. */
	pid_t pid;

	/* Status code. */
	int status;

	pidfd = open (pidfile, O_RDONLY);
	if (pidfd < 0)
	{
		if (errno != ENOENT)
		{
			error ("Error %d opening PID file %s",
				errno, pidfile);
		}
		else
		{
			error ("Cannot find PID file - daemon possibly not running");
		}
	}
	(void) memset (pidbuf, 0, (size_t) PID_MAX);
	status = (int) read (pidfd, pidbuf, (size_t) PID_MAX);
	if (status < 0)
	{
		error ("Error %d reading PID file %s", errno);
	}
	pidbuf[status] = EOS;
	pid = (pid_t) atoi (pidbuf);
	if (pid == (pid_t) 0)
	{
		error ("Garbled PID file '%s'", pidbuf);
	}
	return (pid);
}

/* Poll device. */

static void
qpoll (void)
{
	unsigned int wait;
	int present;

	wait = DEFAULT_POLL_WAIT;
	present = qprobe ();
	while (! present)
	{
		progress (".");
		(void) sleep (wait);
		present = qprobe ();
	}
}

/* Print help. */

static void
print_help (void)
{
	(void) fprintf (stdout, "\
Daemon to feed the random pool with entropy from a\n\
Quantis RNG device.\n\
Usage:\n\
    qrngd [-b bufsize][-n n][-s sleep][-d level][-v][-w][-h]\n\
where\n\
    -b bufsize      to specify buffer size for random bytes.\n\
    -n n            number of buffers to send in one go.\n\
    -s sleep        sleep period in seconds as a floating number.\n\
    -d level        set debug level.\n\
    -v              verbose output.\n\
    -w              wait for the device to be connected.\n\
    -h              print this help.\n\
It will be running as a daemon in the background.\n\
Note that debug level > 16 will disable functionality.\n\
");
	exit (FAILURE);
}

/* Main. */

int
main (int argc, char *argv[])
{

	/* Options string. */
	char *options = "b:d:kn:ps:vwh";

	/* Current option. */
	char opt;

	/* Number of repetitions. */
	int n = 1;

	/* Delay in microseconds. */
	int delay = 1000000;

	/* Eagerly wait for the device or bail out. */
	int wait = false;

	/* Status code. */
	int status;

	/* PID. */
	pid_t pid;

	/* Counter. */
	int i = 0;

	/* Deal with the options. */
	opterr = 0;
	opt = getopt (argc, argv, options);
	while (opt != (char) -1)
	{
		switch (opt)
		{
		case 'b':

			/* Get buffer size. */
			if (optarg == NULL)
			{
				error ("No argument for buffersize");
			}
			buffersize = atoi (optarg);
			if (buffersize == 0)
			{
				error ("Wrong buffer size %s", optarg);
			}
			if (buffersize < 0)
			{
				error ("Negative buffer size");
			}
			if (buffersize > MAX_BUFFER)
			{
				error ("Buffer size is too large");
			}
			break;
		case 'd':

			/* Debug level. */
			if (optarg == NULL)
			{
				error ("No argument for switch -d");
			}
			debug = atoi (optarg);
			if (debug <= 0)
			{
				error ("Debug level is unsuitable");
			}
			break;
		case 'k':

			/* Kill running daemon. */
			dpf ("quitting");
			pid = daemonpid ();
			status = kill (pid, SIGQUIT);
			if (status < 0)
			{
				error ("Error sending quit signal to process %d", (int) pid);
			}

			/* Cleanup. */
			(void) sleep ((unsigned int) 1);
			(void) unlink (pidfile);
			exit (SUCCESS);
			break;
		case 'n':

			/* Get number of buffers to repeat action with. */
			if (optarg == NULL)
			{
				error ("No argument for switch -n");
			}
			n = atoi (optarg);
			if (n <= 0)
			{
				error ("Number of buffers/repetitions is unsuitable");
			}
			break;
		case 'p':

			/* Probe test. */
			dpf ("testprobe");
			verbose = true;
			sig (SIGHUP, defhand);
			sig (SIGINT, defhand);
			sig (SIGQUIT, defhand);
			sig (SIGTERM, defhand);
			while (true)
			{
				if (qprobe ())
				{
					progress ("!");
				}
				else
				{
					progress (".");
				}
				(void) sleep ((unsigned int) 1);
			}
			exit (SUCCESS);
			break;
		case 's':

			/* Sleep time in seconds expressed as a floating point number. */
			if (optarg == NULL)
			{
				error ("No argument for sleeping period");
			}

			/* Converted to microseconds as integer. */
			delay = (int) (atof (optarg) * ((double) 1.0e6));
			if (delay <= 0)
			{
				error ("Sleeping period is unsuitable (%s)", optarg);
			}
			break;
		case 'v':
			verbose = true;
			break;
		case 'w':
			wait = true;
			break;
		case 'h':
			print_help ();
			break;
		case '?':
			error ("Unknown option");
			break;
		default:
			error ("Unkown option, exiting");
			break;
		}
		opt = getopt (argc, argv, options);
	}

	/* Show what we got. */
	if (debug > 0)
	{
		msg ("Debug level is %d", debug);
		msg ("Buffer size is %d", buffersize);
		msg ("Repeat factor is %d", n);
		msg ("Sleep delay is %d ms (%d s)", delay, delay / 1000000);
		msg ("Device name is %s", mixrng_name);
		msg ("PID file name is %s", pidfile);
		msg ("Log file name is %s", logfile);
		msg ("Verbose is %d", verbose);
		msg ("Wait is %d", wait);
	}

	/* Check PID file. */
	dpf ("checkpid");
	checkpid ();

	/* Daemonize, switch to log and create PID file. */
	if (debug <= 8)
	{
		dpf ("daemonize");
		status = daemon (0, 0);
		if (status != 0)
		{
			error ("Daemonize failed with %d", errno);
		}
		switchlog ();
	}
	dpf ("savepid");
	savepid (getpid());

	/* Establish signal handlers. */
	dpf ("signals");
	sig (SIGHUP, defhand);
	sig (SIGINT, defhand);
	sig (SIGQUIT, defhand);
	sig (SIGTERM, defhand);

retry:
	dpf ("initializing");
	vmsg ("Starting up");

	/* Allocate buffer. */
	if (buffersize > MAX_BUFFER)
	{
		error ("Buffer size is too large");
	}
	if (n * buffersize > MAX_MIX)
	{
		error ("Might be too much for /dev/random");
	}
	buffer = (char *) malloc ((size_t) buffersize);
	if (buffer == NULL)
	{
		error ("Not enough memory");
	}
	(void) memset (buffer, 0, buffersize);

	/* Open the mixing device. */
	dpf ("feeding");
	mfd = mixrng_open ();
	if (debug == 0)
	{
		mixrng_nodebug (mfd);
	}

	/* Check if the device is connected and bail out when
	   waiting is not being asked. */
	dpf ("probe");
	if (! qprobe ())
	{
		if (wait)
		{
			dpf ("poll");
			qpoll ();
			(void) sleep ((unsigned int) 1);
		}
		else
		{
			error ("Quantis device not connected");
		}
	}

	/* Open RNG device (global). Disable USB debug. */
	q = qcreate ();
	if (q == NULL)
	{
		error ("Cannot create Quantis device descriptor");
	}
	status = qopen (q);
	if (status < 0)
	{
		qemsg (status, "Error opening device");
		error ("Error opening Quantis device");
	}
	qdebug (0);

	/* Feed the pool. */
	while (true)
	{

		/* Mix in n buffers in a go. */
		dpf ("mixing");
		for (i=0; i<n; i++)
		{
			(void) memset (buffer, 0, buffersize);
			status = qread (q, buffer, buffersize);
			if (status < 0)
			{
				if (! qprobe ())
				{
					if (wait)
					{

						/* Possibly the error was due to disconnect,
						   hope that it will come back. */
						dpf ("poll");
						qpoll ();

						/* Came back online, cleanup and have to give
						   time to settle down. Then retry. */
						dpf ("cleanup");
						cleanup ();
						(void) sleep ((unsigned int) 1);
						dpf ("retry");
						goto retry;
					}
					else
					{
						qemsg (status, "Read error - possibly device not present");
						error ("Read error %d, possibly device not present", status);
					}
				}
				else
				{
					qemsg (status, "Error reading device");
					error ("Cannot read from Quantis device");
				}
			}
			if (debug > 8)
			{
				bufprint (buffer, buffersize);
				(void) printf ("\n");
			}
			if (debug < 16)
			{
				if (status < 0)
				{
					error ("Read status signals error from the device - not mixing, confused");
				}
				else
				{
					mixrng_write (mfd, buffer, buffersize);
				}
			}
		}

		/* Sleep. */
		progress ("!");
		dpf ("sleeping");
		msleep (delay);
	}

	/* Finish. */
	dpf ("surprise");
	cleanup ();
	exit (SUCCESS);
}

/* End of file QRNGD.C. */

