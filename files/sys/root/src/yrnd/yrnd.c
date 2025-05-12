
/* YRND.C obtains randomness from the Yubikey. */

/* The original sample program for PCSC reworked to
   provide access to randomness in the Yubikey. */

/* System includes. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <string.h>

/* PCSC includes. Note that winscard will
   refer an include without the PCSC prefix so
   an another -I should be added like
   -I /usr/local/include/PCSC as well. */
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

/* Status codes. */
#define SUCCESS (0)
#define FAILURE (1)

/* Error status. */
#define ERROR_STATUS (-1)

/* Truth values. */
#ifndef true
#define true ((int) 1)
#endif
#ifndef false
#define false ((int) 0)
#endif

/* Number of random bytes returned from the card.
   Fixed in the protocol. */
#define NRAND 8

/* Maximum number of random bytes to handle in one go. */
#define MAXNBYTES 65536

/* Name of the card size. */
#define NAMELENGTH 255

/* Error exit. */
static void
error (char *msg)
{
	(void) fprintf (stderr, "%s\n", msg);
	exit (ERROR_STATUS);
}

/* Macro to check return status. */
#define scard_check(f, status) \
	if (SCARD_S_SUCCESS != status) \
	{ \
		(void) fprintf (stderr, "%s: %s\n", \
			f, pcsc_stringify_error (status)); \
		return (ERROR_STATUS); \
	}

/* Flag to print just the binary bytes. */
static int binary = false;

/* Flag to print the reader name for verification. */
static int verify = false;

/* Flag to print hex bytes separated by blanks. */
static int separate = false;

/* Return 8 bytes of randomness from the Yubikey. */

static int
yrnd (unsigned char *rnd, char *rn)
{

	/* Status code. */
	LONG status;

	/* SCARD variables as in the sample program,
	   in Hungarian notation, which I don't like. */
	SCARDCONTEXT context;
	LPTSTR readernames;
	SCARDHANDLE card;
	DWORD readers, active_protocol, recvlength;
	SCARD_IO_REQUEST piosendpci;
	BYTE recvbuffer[258];

	/* APDU command SELECT APPLICATION PIV. */
	BYTE cmd1[] =
	{
		0x00, 0xA4, 0x04, 0x00, 0x05,
		0xA0, 0x00, 0x00, 0x03, 0x08
	};

	/* APDU command GENERAL AUTHENTICATE REQUESTING CHALLENGE.
	   8 bytes of randomness will be returned after
	   a four byte header and followed by status code
	   which should be 90 00. */
	BYTE cmd2[] =
	{
		0x00, 0x87, 0x03, 0x9b, 0x04, 0x7c, 0x02, 0x81, 0x00
	};

	/* Counters. */
	unsigned int i;
	unsigned int j;

	/* Initialize. */
	status = SCardEstablishContext (SCARD_SCOPE_SYSTEM, NULL, NULL,
		&context);
	scard_check ("SCardEstablishContext", status);

/* Whan AUTOALLOCATE the SCard function will allocate and
   it has to be freed with the SCard function. Otherwise just
   calloc and free. */
#ifdef SCARD_AUTOALLOCATE
	readers = SCARD_AUTOALLOCATE;
	status = SCardListReaders (context, NULL,
		(LPTSTR) &readernames, &readers);
#else
	status = SCardListReaders (context, NULL, NULL, &readers);
	scard_check ("SCardListReaders", status)
	readernames = calloc (readers, sizeof(char));
	status = SCardListReaders (context, NULL, readernames, &readers);
#endif

	/* Manual check instead of scard_check. */
	if (SCARD_S_SUCCESS != status)
	{
		(void) fprintf (stderr, "%s\n",
			pcsc_stringify_error (status));
		return (ERROR_STATUS);
	}

	/* Reader name is in readernames. Actually it returns names of
	   several readers if more than one, in a strange double EOS
	   terminated format. We have only one reader so we don't bother.
	   When tested the Yubikey returns several "phantom" reader names. */

	/* Return reader name and quit when asked to do so. */
	if (rn != NULL)
	{
		strncpy (rn, (char *) readernames, NAMELENGTH);
		goto quit;
	}

	/* Connect. */
	status = SCardConnect (context, readernames, SCARD_SHARE_SHARED,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &card, &active_protocol);
	scard_check ("SCardConnect", status);

	/* Decide protocol. */
	switch (active_protocol)
	{
	case SCARD_PROTOCOL_T0:
		piosendpci = *SCARD_PCI_T0;
		break;
	case SCARD_PROTOCOL_T1:
		piosendpci = *SCARD_PCI_T1;
		break;
	}

	/* Transmit cmd1. */
	recvlength = sizeof (recvbuffer);
	status = SCardTransmit (card, &piosendpci, cmd1, sizeof(cmd1),
		NULL, recvbuffer, &recvlength);
	scard_check ("SCardTransmit", status);
	/* Response is in recvbuffer. */

	/* Transmit cmd2. */
	recvlength = sizeof (recvbuffer);
	status  = SCardTransmit (card, &piosendpci, cmd2, sizeof(cmd2),
		NULL, recvbuffer, &recvlength);
	scard_check ("SCardTransmit", status)

	/* Ignore first four bytes which is a header.
	   The last two bytes are status, should be 90 00.*/
	j = recvlength - 1;
	if (! (recvbuffer[j-1] == 0x90 &&
		   recvbuffer[j] == 0x00))
	{
		error ("APDU status not 90 00");
	}
	j = 0;

	/* We got 8 bytes, which is NRAND. */
	for(i=4; i<(recvlength-2); i++)
	{
		rnd[j] = recvbuffer[i];
		if (j == NRAND)
		{

			/* Handbreak. */
			error ("Returned APDU is not to format - confused");
		}
		j++;
	}

	/* Finish. Disconnect. */
	status = SCardDisconnect (card, SCARD_LEAVE_CARD);
	scard_check ("SCardDisconnect", status);

quit:
	/* Deallocate and release contect. */
#ifdef SCARD_AUTOALLOCATE
	status = SCardFreeMemory (context, readernames);
	scard_check ("SCardFreeMemory", status);
#else
	free (readernames);
#endif
	status = SCardReleaseContext (context);
	scard_check ("SCardReleaseContext", status)
	return (SUCCESS);
}

/* Print one byte. Binary or hex. */

static void
printb (unsigned char ch)
{
	int status;

	if (binary)
	{
		status = fputc (ch, stdout);
		if (status == EOF)
		{
			error ("fputc failed");
		}
	}
	else
	{
		if (separate)
		{
			(void) fprintf (stdout, "%02X ", ch);
		}
		else
		{
			(void) fprintf (stdout, "%02X", ch);
		}
	}
}

/* Print NRAND random bytes. */

static void
printrnd (unsigned char *rnd)
{
	int i;

	for (i=0; i<NRAND; i++)
	{
		printb (rnd[i]);
	}
}

/* Print nbytes random bytes. */

static void
yrndbytes (int nbytes)
{
	int n;
	int r;
	int i;
	unsigned char rnd[NRAND];
	int status;

	/* Check. */
	if (nbytes < 0)
	{
		error ("Wrong nbytes");
	}
	/* Bytes come in NRAND chunks. */
	n = nbytes / NRAND;
	r = nbytes - (n * NRAND);
	for (i=0; i<n; i++)
	{
		status = yrnd (rnd, NULL);
		if (status < 0)
		{
			error ("Error getting random bytes");
		}
		printrnd (rnd);
	}

	/* Print the remainder (bytes). */
	status = yrnd (rnd, NULL);
	if (status < 0)
	{
		error ("Error getting random bytes");
	}
	for (i=0; i<r; i++)
	{
		printb (rnd[i]);
	}
}

/* Print help. */

static void
printhelp (void)
{
	(void) fprintf (stdout, "\
This small program gets random numbers from the Yubikey.\n\
Usage:\n\
    yrnd [-n n][-b][-h]\n\
where\n\
    -n n            number of bytes to get. The default is 8.\n\
                    Not larger than 32768.\n\
    -b              binary, the default is to print the bytes in hex.\n\
    -s              print the bytes separated with blanks.\n\
    -h              print this help.\n\
");
	exit (FAILURE);
}

/* Main program. */

int
main (int argc, char *argv[])
{

	/* Options. */
	char *options = "bhn:sv";
	char opt;

	/* Number of bytes. */
	int n = 8;

	/* Status code. */
	int status;

	/* The yrnd call requires this. */
	unsigned char cardname[NAMELENGTH];

	/* Deal with the options. */
	opterr = 0;
	opt = getopt (argc, argv, options);
	while (opt != (char) -1)
	{
		switch (opt)
		{
		case 'b':
			binary = true;
			break;
		case 'n':

			/* Get number. */
			if (optarg == NULL)
			{
				error ("No argument number of bytes");
			}
			n = atoi (optarg);
			if (n == 0)
			{
				error ("Wrong n");
			}
			if (n < 0)
			{
				error ("Negative n");
			}
			if (n > MAXNBYTES)
			{
				error ("Too large n");
			}
			break;
		case 's':
			separate = true;
			break;
		case 'v':
			verify = true;
			break;
		case 'h':
			printhelp ();
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

	if (verify)
	{
		status = yrnd (NULL, cardname);
		if (status < 0)
		{
			error ("Error initializing card");
		}

		/* Print card name and quit. */
		(void) printf ("%s\n", cardname);
		exit (SUCCESS);
	}

	/* Get n random bytes. */
	yrndbytes (n);

	/* New line at the end if not binary. */
	if (! binary)
	{
		(void) fprintf (stdout, "\n");
	}
	exit (SUCCESS);
}

/* End of file YRND.C. */

