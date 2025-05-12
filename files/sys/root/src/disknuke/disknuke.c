
/* Simple program to write zeroes where there aren't already. */

/* This program suits OpenBSD or FreeBSD. Not a substitute for proper
   disk sanitizing but better than nothing. It presumes that the disk
   is already mostly zeroes, which is the case with newly bought USB
   sticks. */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/dkio.h>
#include <sys/disklabel.h>

/* Truth. */
#define true 1
#define false 0

/* Default buffer size. 128 4K pages is a block. */
#define BUFFERSIZE 524288

/* Error exit. */

static void
error (char *msg)
{
	(void) fprintf (stderr, "%s\n", msg);
	(void) fflush (stderr);
	exit (1);
}

/* If not all zeroes. */

static int
nonzeroes (char *buf, size_t len)
{
	char *p;
	int found;
	size_t count;
	char ch;

	p = buf;
	found = false;
	count = (size_t) 0;
	while (count < len)
	{
		ch = *p;
		if (ch != '\0')
		{

			/* Overwrite when not zero. */
			*p = '\0';
			found = true;
		}
		count++;
		p++;
	}
	return (found);
}

/* Used to overwrite last progress report. */
#define BLANKOUT "                                          \r"

/* Main program. */

int
main (int argc, char *argv[])
{
	char opt;
	char *options = "b:chvs:e:";
	int bufsize = BUFFERSIZE;
	int verbose = false;
	int progress = false;
	int check = false;
	off_t start = (off_t) -1;
	off_t end = (off_t) -1;
	char *fn;
	char *buf;
	int fd;
	ssize_t nb;
	off_t lastpos;
	off_t endpos;
	off_t startpos;
	off_t seekoff;
	int status;
	struct disklabel dl;
	float percent;
	unsigned long blockswritten;

	/* Arguments and switches. */
	opterr = 0;
	opt = getopt (argc, argv, options);
	while (opt != (char) -1)
	{
		switch (opt)
		{
		case 'b':
			bufsize = atoi (optarg);
			if (bufsize == 0)
			{
				error ("Wrong buffer size");
			}
			break;
		case 'c':
			check = true;
			break;
		case 'h':
			(void) printf ("Usage: disknuke [-h][-v][-b bufsize][-c][-s start][-e end] deviceorfile\n");
			(void) printf ("Note: start and end cannot be zero, the default start is 0. They should be in hex\n");
			exit (1);
			break;
		case 'v':
			verbose = true;
			break;
		case 's':
			start = (off_t) strtol (optarg, NULL, 16);
			if (start == 0)
			{
				error ("Wrong start offset");
			}
			break;
		case 'e':
			end = (off_t) strtol (optarg, NULL, 16);
			if (end == 0)
			{
				error ("Wrong end offset");
			}
			break;
		case '?':
			error ("Missig argument or wrong switch");
			break;
		default:
			error ("Wrong switch");
			break;
		}
		opt = getopt (argc, argv, options);
	}
	if ((argc - optind) != 1)
	{
		error ("Specify exactly one argument, the device/file name");
	}
	else
	{

		/* We got exactly one argument which is the file/device name. */
		fn = argv[optind];
	}

	/* Warning. */
	if (verbose)
	{
		if (! check)
		{
			(void) printf ("%s will be nuked in 5 seconds - press CTRL/C now if not sure\n", fn);
			(void) sleep (5);
		}
		else
		{
			(void) printf ("Check only for %s\n", fn);
		}
	}
	else
	{
		(void) sleep (3);
	}

	/* Buffer. */
	buf = (char *) malloc ((size_t) bufsize);
	if (buf == NULL)
	{
		error ("Cannot allocate buffer");
	}

	/* Open the file. */
	fd = open (fn, O_RDWR);
	if (fd < 0)
	{
		error ("Cannot open device");
	}

	/* Get the size. */
	endpos = lseek (fd, (off_t) 0, SEEK_END);
	if (endpos < 0)
	{
		error ("Function lseek failed getting end position");
	}
	else if (endpos == (off_t) 0)
	{

		/* Probably we got a disk device file specified. */
		status = ioctl (fd, DIOCGDINFO, &dl);
		if (status < 0)
		{
			error ("Function ioctl failed");
		}

		/* We should have disk size now from disk label. This will work
		   with BSDs. */
		endpos = (off_t) (((unsigned long) dl.d_ncylinders) * 
			((unsigned long) dl.d_secpercyl) *
			((unsigned long) dl.d_secsize));
	}
	else
	{

		/* This should be a file we got size, make sure we start at the top. */
		startpos = lseek (fd, (off_t) 0, SEEK_SET);
		if (startpos < 0)
		{
			error ("Function lseek failed setting start position");
		}
	}
	blockswritten = 0;

	/* Print size. */
	if (verbose)
	{
		(void) printf ("%s is %lu bytes (%lu MB)\n", fn,
			(unsigned long) endpos, (unsigned long) (endpos / 1048576));
	}

	/* Start offset specified, seek. */
	if (start != (off_t) -1)
	{
		lastpos = lseek (fd, (off_t) start, SEEK_SET);
		if (lastpos < 0)
		{
			error ("Function lseek failed setting start offset");
		}
	}

	/* Remember last position in case we need to go back then read. */
	lastpos = lseek (fd, (off_t) 0, SEEK_CUR);
	if (lastpos < 0)
	{
		error ("Function lseek failed saving position");
	}
	nb = read (fd, buf, bufsize);
	if (nb < 0)
	{
		error ("Error reading device");
	}
	while (nb != (ssize_t) 0)
	{

		/* Suppose the device is already mostly zeroes. */
		if (nonzeroes (buf, nb))
		{

			/* Non-zero, backtrack and write it back. */
			if (! check)
			{
				seekoff = lseek (fd, lastpos, SEEK_SET);
				if (seekoff < 0)
				{
					error ("Function lseek failed backing up");
				}
				if (write (fd, buf, nb) != nb)
				{
					error ("Issue writing buffer back");
				}
			}
			else
			{
				percent = ((float) lastpos * (float) 100.0) / ((float) endpos);
				(void) printf ("%016lx %6.2f*\n", (unsigned long) lastpos, percent);
				(void) fflush (stdout);
			}
			blockswritten++;
		}

		/* Quit when end position reached. */
		if (end != (off_t) -1)
		{
			if ((lastpos + (off_t) bufsize) > end)
			{
				if (verbose)
				{
					if (progress)
					{
						(void) printf (BLANKOUT);
					}
					(void) printf ("End reached at %016lx\n", (unsigned long) lastpos);
					(void) printf ("Note that last partial buffer (%lld bytes) is not done\n", endpos - lastpos);
				}
				break;
			}
		}

		/* Show position. */
		if (verbose)
		{
			percent = ((unsigned long) lastpos * (unsigned long) 100) / ((unsigned long) endpos);

			/* Note that you need to use fflush otherwise the line will be buffered
			   until a newline comes. */
			(void) printf ("%016lx %6.2f (%12ld MB)\r", (unsigned long) lastpos, (float) percent, (unsigned long) (lastpos / 1048576) );
			(void) fflush (stdout);
			progress = true;
		}

		/* Again. */
		lastpos = lseek (fd, (off_t) 0, SEEK_CUR);
		if (lastpos < 0)
		{
			error ("Function lseek failed saving position");
		}
		nb = read (fd, buf, bufsize);
		if (nb < 0)
		{
			error ("Error reading device");
		}
	}

	/* Finish. */
	free (buf);
	if (close (fd) < 0)
	{
		error ("Error closing device");
	}
	if (verbose)
	{
		if (progress)
		{
			(void) printf (BLANKOUT);
			(void) printf ("Finished\n");
		}
		else
		{
			(void) printf ("Finished\n");
		}
		if (! check)
		{
			(void) printf ("%lu blocks written\n", blockswritten);
		}
		else
		{
			(void) printf ("%lu blocks dirty\n", blockswritten);
		}
	}
	exit (0);
}

