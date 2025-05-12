
/* USEROK.C is a test program to try authenticator routines. */

/* System include files. */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <login_cap.h>
#include <bsd_auth.h>

/* Error exit. */

static void
error (char *msg)
{
	(void) fprintf (stderr, "%s\n", msg);
	exit (1);
}

int
main (int argc, char *argv[])
{
	login_cap_t *c;
	char *s;
	int status;

	c = login_getclass ("sequence");
	if (c == NULL)
	{
		error ("Class not found");
	}

	s = login_getstyle (c, "seq", "login");
	if (s == NULL)
	{
		error ("Style not found");
	}
	login_close (c);

	status = auth_userokay ("test", "seq", "login", NULL);
	if (status == 0)
	{
		error ("Failure");
	}
	else
	{
		(void) printf ("Ok\n");
	}
	exit (0);
}

/* End of file USEROK.C. */

