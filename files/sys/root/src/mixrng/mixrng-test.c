
/* MIXRNG-TEST.C is a test program for mixrng. */

/* System include files. */
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

/* Our header from under the srouce tree. */
#include "/usr/src/sys/dev/mixrngio.h"

/* Status codes. */
#define SUCCESS 0
#define FAILURE 1

/* Boolean values. */
#ifndef true
#define true ((int) 1)
#endif
#ifndef false
#define false ((int) 0)
#endif

/* Buffer size. */
#define DEFAULT_BUFSIZE 512

/* Silent. */
static int silent = false;

/* Quit with error. */

static void
error (const char *format, ...)
{
	va_list args;

	va_start (args, format);
	(void) vfprintf (stderr, format, args);
	va_end (args);
	(void) fprintf (stderr, "\n");
	exit (FAILURE);
}

/* Print message with newline. */

static void
msg (const char *format, ...)
{
	va_list args;

	if (! silent)
	{
		va_start (args, format);
		(void) vprintf (format, args);
		va_end (args);
		(void) printf ("\n");
	}
}

/* Print buffer. */

static void
printbuf (void *buf, size_t len)
{
	unsigned char *charbuf;
	long int i;
	unsigned char ch;

	if (! silent)
	{
		charbuf = (unsigned char *) buf;
		for (i=0; i<(long int) len; i++)
		{
			ch = charbuf[i];
			(void) printf ("%02x", ch);
		}
		(void) printf ("\n");
	}
}

/* Print help. */

static void
printhelp (void)
{
	printf ("\n\
This is a test program for mixrng.\n\
Usage:\n\
    mixrng-test [-h][-b bufsize][-n n][-s] test...\n\
where\n\
    -b bufsize  size of the buffer to use.\n\
    -n n        number of repetitions.\n\
    -s          silent.\n\
test is\n\
    open        open device\n\
    read        read buffers\n\
    write       write buffers\n\
    ioctl       test ioctl\n\
    debug       switch on debug\n\
    nodebug     switch off debug\n\
    stat        print statistics in syslog\n\
    close       close device\n\
Note: Read would work only with the debug version\n\
of the driver.\n\
");
	exit (FAILURE);
}

/* Main program. */

int
main (int argc, char *argv[])
{
	char *options = "b:n:sh";
	char ch;
	size_t bufsize = (size_t) DEFAULT_BUFSIZE;
	int n = 1;
	char *buf = NULL;
	char *cmd;
	int fd = -1;
	int status;
	int i;
	struct mixrng_params params;

	/* Switches. */
	opterr = 0;
	ch = getopt (argc, argv, options);
	while (ch != -1)
	{
		switch (ch)
		{
		case 'b':

			/* Buffersize. */
			bufsize = (size_t) atol (optarg);
			if (bufsize == (size_t) 0)
			{
				error ("Wrong buffersize");
			}
			break;
		case 'n':

			/* Number of repetitions. */
			n = atoi (optarg);
			if (n == 0)
			{
				error ("Wrong n");
			}
			break;
		case 's':

			/* Silent. */
			silent = true;
			break;
		case 'h':
			printhelp ();
			break;
		case '?':
		default:
			error ("Wrong switch");
			break;
		}
		ch = getopt (argc, argv, options);
	}

	/* Allocate buffer. */
	buf = (char *) malloc (bufsize);
	if (buf == NULL)
	{
		error ("Not enough memory");
	}
	(void) memset (buf, 0, bufsize);

	/* Process options. */
	cmd = argv[optind];
	while (optind < argc)
	{
		if (strcmp (cmd, "open") == 0)
		{

			/* Open. */
			msg ("open");
			fd = open ("/dev/mixrng", O_RDWR, 0);
			if (fd < 0)
			{
				if (errno == EPERM)
				{

					/* No permission, production version does not
					   allow reads. Try write only. */
					fd = open ("/dev/mixrng", O_WRONLY, 0);
					if (fd < 0)
					{

						/* Still failed. */
						error ("Error %d opening device", errno);
					}
				}
				else
				{

					/* Other error. */
					error ("Error %d opening device", errno);
				}
			}
		}
		else if (strcmp (cmd, "read") == 0)
		{

			/* Buffer read. */
			msg ("Read");
			if (fd == -1)
			{
				error ("Device not open");
			}
			for (i=0; i<n; i++)
			{
				status = read (fd, buf, bufsize);
				if (status < 0)
				{
					error ("Error %d reading device", errno);
				}
				msg ("%4d %d '%s'\n", i, status, buf);
				printbuf (buf, bufsize);
			}
		}
		else if (strcmp (cmd, "write") == 0)
		{

			/* Buffer write. */
			msg ("write");
			if (fd == -1)
			{
				error ("Device not open");
			}
			for (i=0; i<n; i++)
			{
				(void) memset (buf, i, bufsize);
				status = write (fd, buf, bufsize);
				if (status < 0)
				{
					error ("Error %s writing device", status);
				}
			}
		}
		else if (strcmp (cmd, "ioctl") == 0)
		{

			/* Ioctl test. */
			msg ("ioctl");
			if (fd == -1)
			{
				error ("Device not open");
			}
			params.code = 101;
			strncpy (params.data, "Test data", MIXRNGIO_DATA_LENGTH);
			status = ioctl (fd, MIXRNGIO_TEST, &params);
			if (status < 0)
			{
				error ("Function ioctl failed with %d", errno);
			}
			params.code = 99;
			status = ioctl (fd, MIXRNGIO_DEBUG, &params);
			if (status < 0)
			{
				error ("Function ioctl failed with %d", errno);
			}
		}
		else if (strcmp (cmd, "debug") == 0)
		{

			/* Switch debug on. */
			msg ("debug");
			if (fd == -1)
			{
				error ("Device not open");
			}
			params.code = 99;
			status = ioctl (fd, MIXRNGIO_DEBUG, &params);
			if (status < 0)
			{
				error ("Function ioctl failed with %d", errno);
			}
		}
		else if (strcmp (cmd, "nodebug") == 0)
		{

			/* Switch debug off. */
			msg ("nodebug");
			if (fd == -1)
			{
				error ("Device not open");
			}
			params.code = 0;
			status = ioctl (fd, MIXRNGIO_DEBUG, &params);
			if (status < 0)
			{
				error ("Function ioctl failed with %d", errno);
			}
		}
		else if (strcmp (cmd, "stat") == 0)
		{

			/* Statistics in syslog. */
			msg ("stat");
			if (fd == -1)
			{
				error ("Device not open");
			}
			status = ioctl (fd, MIXRNGIO_STAT, &params);
			if (status < 0)
			{
				error ("Function ioctl failed with %d", errno);
			}
		}
		else if (strcmp (cmd, "close") == 0)
		{

			/* Close. */
			msg ("close");
			if (fd == -1)
			{
				error ("Device not open");
			}
			if (buf == NULL)
			{
				error ("No buffer");
			}
			status = close (fd);
			if (status < 0)
			{
				error ("Error %d closing device", errno);
			}
		}
		else
		{
			error ("Unknown command %s", cmd);
		}
		optind++;
		cmd = argv[optind];
	}

	/* Finish. */
	(void) memset (buf, 0, bufsize);
	free (buf);
	buf = NULL;
	exit (0);
}

/* End of file MIXRNG-TEST.C. */

