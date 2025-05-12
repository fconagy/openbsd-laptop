
/* Read hex dump bytes and convert to binary. */

/* System include. */
#include <stdio.h>
#include <stdlib.h>

/* Read next hex byte. */

static int
nexthex (char *format, unsigned int *value)
{
	int status;

	status = scanf (format, value);
	if (status != 1)
	{
		if (status == 0)
		{

			/* Cannot convert. */
			fprintf (stderr, "Conversion error\n");
			exit (1);
		}
		if (status == EOF)
		{
			if (ferror (stdin))
			{

				/* There was an actual error. */
				fprintf (stderr, "Error reading stdin");
				exit (1);
			}
		}
	}

	/* Either 1 or EOF at this point. */
	return (status);
}

/* Main program. */

int
main (int argc, char *argv[])
{
	int status;
	unsigned int value;
	char *format = "%02X";
	char *binary = "%c";

	/* Note that it will work only with uppercase hex. */
	status = nexthex (format, &value);
	while (status != EOF)
	{
		printf (binary, value);
		status = nexthex (format, &value);
	}
	exit (0);
}

