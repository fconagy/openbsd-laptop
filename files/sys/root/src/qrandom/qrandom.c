
/* File QRANDOM.C is a small Quantis random number generator utility. */

/* Utility to extract random numbers from a Quantis USB hardware
   random number generator device. Based on Quantis example programs. */

/* System includes. */
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <fcntl.h>
#include <sys/stat.h>

/* LIBUSB library header file. It is
   under /usr/local/include. */
#include <libusb-1.0/libusb.h>

/* Our Quantis include. */
#include "qusb.h"

/* Buffer size. */
#define MAX_BUFFER 100000

/* One megabyte. */
#define MIB 1048576

/* Maximum amout to mix into /dev/random. */
#define MAX_MIX (2 * MIB)

/* Debug. */
static int debug = 0;

/* Global QRNG device descriptor. */
qdev_t *q = NULL;

/* Global mixrng device file name. */
static char *mixrng_name = "/dev/mixrng";

/* Syllable table size. */
#define STABSIZE (26 * 5)

/* Table of syllables. */
static char *stab[] =
{
	"a", "i", "u", "e", "o",
	"a", "e", "e", "e", "i",
	"r", "s", "t", "b", "z",
	"ka", "ki", "ku", "ke", "ko",
	"sa", "si", "su", "se", "so",
	"ta", "ti", "tu", "te", "to",
	"na", "ni", "nu", "ne", "no",
	"ha", "hi", "fu", "he", "ho",
	"ma", "mi", "mu", "me", "mo",
	"ya", "yi", "yu", "ye", "yo",

	"ra", "ri", "ru", "re", "ro",
	"wa", "wi", "wu", "we", "wo",
	"ga", "gi", "gu", "ge", "go",
	"za", "zi", "zu", "ze", "zo",
	"da", "di", "du", "de", "do",
	"ba", "bi", "bu", "be", "bo",
	"pa", "pi", "pu", "pe", "po",
	"ca", "ci", "cu", "ce", "co",
	"fa", "fi", "fu", "fe", "fo",
	"ja", "ji", "ju", "je", "jo",

	"la", "li", "lu", "le", "lo",
	"pa", "pi", "pu", "pe", "po",
	"qa", "qi", "qu", "qe", "qo",
	"va", "vi", "vu", "ve", "vo",
	"xa", "xi", "xu", "xe", "xo",
	"za", "zi", "zu", "ze", "zo",
	NULL
};

/* Error exit. */

static void
error (char *msg)
{
	(void) fprintf (stderr, "%s\n", msg);
	qclose (q);
	qdestroy (q);
	exit (FAILURE);
}

/* Display bytes. */

static void
bufprint (int options, char *buffer, int nbytes)
{
	int i;
	char *f1 = "%02X";
	char *f2 = "%02x";
	char *f3 = "%02X ";
	char *f4 = "%02x ";
	char *format;
	int last;

	/* Decide on format. Print options are in the header file. */
	format = f2;
	if (options & UPPERCASE)
	{
		if (options & SPACEDELIM)
		{
			format = f3;
		}
		else
		{
			format = f1;
		}
	}
	else
	{
		if (options & SPACEDELIM)
		{
			format = f4;
		}
		else
		{
			format = f2;
		}
	}
	last = nbytes - 1;
	for (i=0; i<nbytes; i++)
	{

		/* Do not put the last space when delimiters. */
		if (options & SPACEDELIM)
		{
			if (i == last)
			{
				if (format == f3)
				{
					format = f1;
				}
				if (format == f4)
				{
					format = f2;
				}
			}
		}

		/* Print one byte in hex format. */
		(void) printf (format, (unsigned char) buffer[i]);
	}
}

/* Check user. */

static void
check_user (void)
{
	char *doasuser;

	/* Check if we are running from under doas.
	   Some functions should be used only running
	   as root for real. */
	doasuser = getenv ("DOAS_USER");
	if (doasuser != NULL)
	{

		/* Running from under doas, not natively. */
		error ("Not root");
	}
}

/* Check if buffersize got a proper value. */

static void
check_buffersize (int buffersize)
{
	if (buffersize == 0)
	{
		error ("Buffer size to use need to be specified");
	}
	if (buffersize < 0)
	{
		error ("Negative buffer size");
	}
	if (buffersize > MAX_BUFFER)
	{
		error ("Buffer size is too large");
	}
}

/* Called when unbuffered options is not suitable. */

static void
check_unbuffered (int unbuffered)
{
	if (unbuffered)
	{
		error ("The -u switch is not suitable for this case");
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
		(void) fprintf (stderr, "Error %d opening %s\n", errno, mixrng_name);
		error ("Cannot open mixrng device file");
	}
	return (fd);
}

/* Write one record to mixrng. */

static void
mixrng_write (int fd, char *buffer, int buffersize)
{
	ssize_t status;

	status = write (fd, (void *) buffer, (size_t) buffersize);
	if (status != (ssize_t) buffersize)
	{
		(void) fprintf (stderr, "Error %d writing %s\n", errno, mixrng_name);
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
		(void) fprintf (stderr, "Error %d closing device file\n", errno);
		error ("Error closing mixrng device");
	}
}

/* Print random bytes. */

static void
qprintbuf (int n, int printopt, qdev_t *q, char *buffer, int buffersize)
{
	int i;
	int status;

	/* Print random bytes on standard output in hex format. */
	for (i=0; i<n; i++)
	{
		status = qread (q, buffer, buffersize);
		if (status < 0)
		{
			qemsg (status, "Error reading device");
			error ("Error reading from Quantis device");
		}
		bufprint (printopt, buffer, buffersize);
		(void) printf ("\n");
	}
}

/* Print help. */

static void
print_help (void)
{
	(void) fprintf (stdout, "\
This program reads random bytes from a Quantis USB device.\n\
Usage\n\
    qrandom [-b nbytes][-n n][-f file][-s][-u][-p][-m][-u][-i lower,upper]\n\
        [-d][-c type][-l]\n\
where\n\
    -b nbytes       to specify buffer size for random bytes.\n\
    -n n            how many buffers of random bytes to obtain.\n\
                    or the number of repetitions.\n\
                    The default is one.\n\
    -o file         name of the output file to use.\n\
                    Use minus for standard output.\n\
                    Works only with bytes.\n\
                    (Root only.)\n\
    -s              use space delimiters\n\
    -U              in uppercase.\n\
    -p              print bytes to standard output in hex format.\n\
    -m              mix bytes to the entropy pool via mixrng.\n\
    -u              unbuffered indirect reads, one packet for one item.\n\
    -i lower,upper  print n integer numbers between lower and upper.\n\
                    range including range.\n\
    -d              print n double floating numbers between 0 and 1.\n\
    -c type         print random character strings of length nbytes.\n\
                    The type can be:\n\
                    1 for printable ASCII characters\n\
                    2 for ASCII letters\n\
                    3 for numbers\n\
                    4 will print syllables\n\
                    5 will print hex bytes\n\
                    6 will print uppercase hex bytes\n\
                    7 just bytes as they are\n\
                    The default type is 5.\n\
    -l              list information about connected device.\n\
                    (Root only.)\n\
    -h              print this help.\n\
Note that the order of switches is significant since some of them\n\
are interpreted as actions.\n\
");
	exit (FAILURE);
}

/* Main. */

int
main (int argc, char *argv[])
{

	/* Status code. */
	int status = -99;

	/* Buffer for the returned random bytes. */
	char *buffer = NULL;

	/* Buffer size in bytes. */
	int buffersize = 0;

	/* Number of repetitions. */
	int n = 1;

	/* Output file name. */
	char *filename = NULL;

	/* Counters. */
	int i = 0;
	int j = 0;

	/* Opened file handle. */
	int fd;

	/* Device file handle. */
	int mfd;

	/* Print options. */
	int printopt = 0;

	/* Unbuffered. */
	int unbuffered = false;

	/* Integer range. */
	char *range = NULL;

	/* Position of comma. */
	char *cpos = NULL;

	/* First and second parts of a range. */
	char *first;
	char *second;
	int ifirst;
	int isecond;

	/* Random integer scaled within a range. */
	int randint;

	/* Random double float. */
	double randdouble;

	/* Character type. */
	int chartype;

	/* Random syllables. */
	int syllables = false;

	/* Hex. */
	int hex = false;
	int bighex = false;

	/* Just bytes. */
	int bytes = false;

	/* Current option. */
	char opt;

	/* Options string. */
	char *options = "D:b:n:o:sUpmui:dc:lh";

	/* Open device and allocate buffer. */
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
	buffer = (char *) malloc (MAX_BUFFER);
	if (buffer == NULL)
	{
		error ("Not enough memory for the buffer");
	}

	/* Disable debug and establish signal handling. */
	qdebug (0);
	qsignal (q, NULL);

	/* Deal with the options. Interpret them as they are coming. */
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
			check_buffersize (buffersize);
			break;
		case 'D':

			/* This should not be used from under doas. */
			check_user ();

			/* Debug. */
			if (optarg == NULL)
			{
				error ("Debug level should be specified");
			}
			debug = atoi (optarg);
			if (debug <= 0)
			{
				error ("Wrong debug level");
			}

			/* Set debug level. */
			qdebug (debug);
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
				error ("Number of buffers/repetitions specified is unsuitable");
			}
			if (n > 1048576)
			{
				error ("Number of buffers/repetitions specified is unreasonable");
			}
			break;
		case 'o':

			/* This should not be used from under doas. */
			check_user ();

			/* We should have buffer size specified already. */
			check_buffersize (buffersize);

			/* Would not work unbuffered. */
			check_unbuffered (unbuffered);

			/* Name of output file. */
			filename = optarg;
			if (filename == NULL)
			{
				error ("File name is null");
			}

			/* Open file. */
			if (strcmp (filename, "-") == 0)
			{

				/* Output file is stdout. */
				fd = 1;
			}
			else
			{

				/* Open file accessible only to the user, create if it's not there. */
				fd = open (filename, O_WRONLY|O_CREAT|O_EXCL|O_EXLOCK, S_IRUSR|S_IWUSR);
				if (fd < 0)
				{
					error ("Error opening output file");
				}
			}

			/* Write n buffers from Quantis into the file. */
			for (i=0; i<n; i++)
			{
				status = qread (q, buffer, buffersize);
				if (status < 0)
				{
					qemsg (status, "Error reading device");
					error ("Cannot read from Quantis device");
				}
				status = (int) write (fd, buffer, (size_t) buffersize);
				if (status != buffersize)
				{
					error ("Error writing output file");
				}
			}

			/* Finish. */
			status = close (fd);
			if (status < 0)
			{
				error ("Error closing output file");
			}
			break;
		case 's':
			printopt |= SPACEDELIM;
			break;
		case 'U':
			printopt |= UPPERCASE;
			break;
		case 'p':

			/* We should have buffer size set. */
			check_buffersize (buffersize);

			/* Not compatible. */
			check_unbuffered (unbuffered);

			/* Print random bytes on standard output in hex format. */
			qprintbuf (n, printopt, q, buffer, buffersize);
			break;
		case 'm':

			/* This should not be used from under doas. */
			check_user ();

			/* We should have a proper buffer size at this point. */
			check_buffersize (buffersize);

			/* Incompatible. */
			check_unbuffered (unbuffered);

			/* Mix to the entropy pool using random_harvest via mixrng. */
			if (n * buffersize > MAX_MIX)
			{
				error ("Might be too much for /dev/random");
			}
			mfd = mixrng_open ();
			for (i=0; i<n; i++)
			{
				status = qread (q, buffer, buffersize);
				if (status < 0)
				{
					qemsg (status, "Error reading device");
					error ("Cannot read from Quantis device");
				}
				mixrng_write (mfd, buffer, buffersize);
			}
			mixrng_close (mfd);
			break;
		case 'u':

			/* This is to use the unbuffered indirect version.
			   This does a full bulk packet read for each byte,
			   chooses a random index into the packet and returns
			   that byte. So that's an indirection in a way. */
			unbuffered = true;
			break;
		case 'i':

			/* Gat scaled random number in a range. */
			if (optarg == NULL)
			{
				error ("No value for option -i - confused");
			}
			range = strdup (optarg);
			if (range == NULL)
			{
				error ("Not enough memory");
			}

			/* We have got a copy of the option argument to write. */
			cpos = index (range, ',');
			if (cpos == NULL)
			{
				error ("Range specification is unsuitable");
			}
			*cpos = '\0';
			cpos++;
			first = range;
			second = cpos;
			ifirst = atoi (first);
			if (ifirst <= 0)
			{
				error ("First number for range is unsuitable");
			}
			isecond = atoi (second);
			if (ifirst <= 0)
			{
				error ("Second number for range is unsuitable");
			}
			if (ifirst >= isecond)
			{
				error ("Wrong numbers for range");
			}
			for (i=0; i<n; i++)
			{
				status = qscaledint (q, unbuffered, &randint, ifirst, isecond);
				if (status < 0)
				{
					qemsg (status, "Error getting scaled int");
					error ("Error getting scaled int from Quantis USB device");
				}
				(void) printf ("%12d", randint);
				(void) printf ("\n");
			}
			free (range);
			break;
		case 'd':

			/* Double between 0 and 1. */
			for (i=0; i<n; i++)
			{
				status = qscaleddouble (q, unbuffered, &randdouble, (double) 0.0, (double) 1.0);
				if (status < 0)
				{
					qemsg (status, "Error getting scaled double");
					error ("Error reading scaled double from Quantis USB device");
				}
				(void) printf ("%14.12f", randdouble);
				(void) printf ("\n");
			}
			break;
		case 'c':

			/* Check. */
			if (optarg == NULL)
			{

				/* The default type. */
				chartype = 5;
			}
			else
			{

				/* Set parameters accordingly to character type.
				   Use various formats. */
				chartype = atoi (optarg);
				if (chartype <= 0)
				{
					error ("Wrong character type");
				}
				switch (chartype)
				{
				case 1:

					/* All printable ASCII characters. */
					ifirst = 33;
					isecond = 126;
					break;
				case 2:

					/* Letters (7-bit ASCII). */
					ifirst = 97;
					isecond = 122;
					break;
				case 3:

					/* Numbers. */
					ifirst = 48;
					isecond = 57;
					break;
				case 4:

					/* Print syllables, easy to pronounce.
					   Get an index into a table of syllables. */
					syllables = true;
					ifirst = 0;
					isecond = STABSIZE - 1;
					break;
				case 5:

					/* Hex byte. */
					hex = true;
					ifirst = 0;
					isecond = 255;
					break;
				case 6:

					/* Hex byte uppercase. */
					bighex = true;
					ifirst = 0;
					isecond = 255;
					break;
				case 7:

					/* Bytes. */
					bytes = true;
					ifirst = 0;
					isecond = 255;
					break;
				case 0:
					error ("Wrong character type number");
					break;
				default:
					error ("Wrong character type id number");
					break;
				}
			}

			/* Buffersize should have a proper value. */
			check_buffersize (buffersize);

			/* Print n lines. */
			for (i=0; i<n; i++)
			{
				for (j=0; j<buffersize; j++)
				{
					status = qscaledint (q, unbuffered, &randint, ifirst, isecond);
					if (status < 0)
					{
							qemsg (status, "Error reading scaled int");
							error ("Error getting scaled int after multiple retries");
					}
					if (syllables)
					{

						/* Print it as syllables. */
						if (randint >= STABSIZE || randint < 0)
						{
							(void) fprintf (stderr, "STABSIZE: %d range: %d..%d, randint: %d\n",
								STABSIZE, ifirst, isecond, randint);
							error ("We have got an issue - confused");
						}
						(void) printf ("%s", stab[randint]);
					}
					else if (hex)
					{

						/* Print it as hex byte. */
						(void) printf ("%02x", (unsigned char) randint);
					}
					else if (bighex)
					{

						/* Print it as uppercase hex byte. */
						(void) printf ("%02X", (unsigned char) randint);
					}
					else if (bytes)
					{

						/* Just bytes as they are. */
						(void) printf ("%c", (unsigned char) randint);
					}
					else
					{

						/* This is letters, numbers, printable ASCII
						   and binary. */
						(void) printf ("%c", (unsigned char) randint);
					}
				}
				(void) printf ("\n");
			}
			break;
		case 'l':

			/* List information about the device. */
			if (q->opened)
			{
				(void) printf("Device: present\n");
			}
			check_user ();
			if (q->manufacturer != NULL &&
				q->serial != NULL)
			{
				(void) printf("Manufacturer: %s\n", q->manufacturer);
				(void) printf("Serial number: %s\n", q->serial);
				(void) printf("Board version: 0x%08X\n", q->version);
			}
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
	if (argv[optind] != NULL)
	{
		error ("Argument will be ignored");
	}

	/* Finish. */
	qclose (q);
	qdestroy (q);
	exit (SUCCESS);
}

/* End of file QRANDOM.C. */

