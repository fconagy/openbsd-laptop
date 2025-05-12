
/* USLEEP.C is a wrapper for the usleep call. */

/* System include files. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* Print help. */

void
printhelp (void)
{
	(void) printf ("Usage: usleep microseconds\n");
	exit (0);
}

int
main (int argc, char *argv[])
{
	int status;
	char *ms;
	int u;

	/* Check. */
	if (argc < 2)
	{
		(void) fprintf (stderr, "Specify sleep time in microseconds\n");
		exit (1);
	}
	ms = argv[1];
	if (strcmp (ms, "-h") == 0)
	{
		printhelp ();
	}
	u = atoi (ms);
	status = usleep ((useconds_t) u);
	if (status == -1)
	{
		(void) fprintf (stderr, "Function usleep returned %d\n", errno);
		exit (1);
	}
	exit (0);
}

/* End of file USLEEP.C. */

