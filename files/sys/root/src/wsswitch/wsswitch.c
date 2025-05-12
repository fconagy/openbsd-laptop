
/* Switch consoles on OpenBSD. */

/* System include files. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/* OpenBSD console subsystem. */
#include <dev/wscons/wsdisplay_usl_io.h>

/* Main, argument is the number of the console terminal, 1-8. */

int main(int argc, char *argv[])
{
	int screen;
	int fd;
	int status;

	if (argv[1] == NULL)
	{
		fprintf (stderr, "Specify screen number\n");
		exit (1);
	}

	/* Get screen number. */
	screen = atoi (argv[1]);
	if (screen < 1 || screen > 8)
	{
		fprintf (stderr, "Screen number invalid\n");
		exit (1);
    }

	/* Open control. */
	fd = open("/dev/ttyCcfg", O_RDWR);  
	if (fd < 0)
	{
		(void) fprintf (stderr, "Error %d opening controlling terminal\n", errno);
		exit (1);
	}

	/* Switch. */
	status = ioctl (fd, VT_ACTIVATE, screen);
	if (status < 0)
	{
		(void) fprintf (stderr, "IOCTL Error %d\n", errno);
		(void) close (fd);
		exit (1);
	}

	/* Success. */
	(void) close (fd);
	exit (0);
}

