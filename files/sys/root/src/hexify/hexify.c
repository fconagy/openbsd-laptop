
/* Read binary bytes and print it in hex. */

/* System include files. */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

/* Truth. */
#ifndef true
#define true ((int) 1)
#endif
#ifndef false
#define false ((int) 0)
#endif

/* Return next character with error checking. */

static int
nextchar ()
{
	int c;

	c = getchar ();
	if (c == EOF)
	{
		if (ferror (stdin))
		{
			fprintf (stderr, "Error %d reading stdin\n", errno);
			exit (1);
		}
	}
	return (c);
}

/* Main program. */

int
main (int argc, char *argv[])
{
	char *options = "ln:u";
	char opt;
	int c;
	char *formatu = "%02X ";
	char *formatl = "%02x ";
	char *formatlastu = "%02X\n";
	char *formatlastl = "%02x\n";
	char *format;
	char *formatlast;
	int last;
	int nitems;
	int n;

	/* Options. */
	nitems = 16;
	format = formatu;
	formatlast = formatlastu;
	opterr = 0;
	opt = getopt (argc, argv, options);
	while (opt != (char) -1)
	{
		switch (opt)
		{
		case 'n':

			/* The number of items on a line. */
			nitems = atoi (optarg);
			if (nitems == 0)
			{
				fprintf (stderr, "Wrong argument to -n\n");
				exit (1);
			}
			break;
		case 'u':
			format = formatu;
			formatlast = formatlastu;
			break;
		case 'l':
			format = formatl;
			formatlast = formatlastl;
			break;
		case '?':
			fprintf (stderr, "Missing argument or unknown switch\n");
			exit (1);
			break;
		default:
			fprintf (stderr, "Wrong switch\n");
			exit (1);
			break;
		}
		opt = getopt (argc, argv, options);
	}

	last = false;
	n = 0;
	c = nextchar ();
	n++;
	while (! feof (stdin))
	{
		if (n >= nitems)
		{
			printf (formatlast, c);
			n = 0;
			last = true;
		}
		else
		{
			printf (format, c);
			last = false;
		}
		n++;
		c = nextchar ();
	}
	if (! last)
	{
		printf ("\n");
	}
	exit (0);
}

