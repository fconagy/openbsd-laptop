
/* LOGIN_ECHO.C is an authentication style which just
   shows the arguments in syslog. It always fails. */

/* Includes. */
#include <sys/types.h>
#include <sys/resource.h>
#include <errno.h>
#include <pwd.h>
#include <readpassphrase.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <util.h>
#include <login_cap.h>
#include <stdarg.h>
#include <syslog.h>
#include <fcntl.h>

/* Error codes. */
#define SUCCESS 0
#define FAILURE 1

/* Globals. */

/* Program name. */
static char *ident = "echo";

/* Back channel file. */
static FILE *back;

/* Error exit. */

static void
error (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vsyslog (LOG_ERR, format, args);
	va_end (args);

	/* Quit. */
	exit (FAILURE);
}

/* Log message. */

static void
log (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vsyslog (LOG_INFO, format, args);
	va_end (args);
}

static void
nl (void)
{
	(void) fprintf (back, BI_SILENT "\n");
}

static void
fail (char *msg)
{
	(void) fprintf (back, BI_VALUE " errormsg %s\n", msg);
	(void) fprintf (back, BI_REJECT "\n");
	exit (FAILURE);
}

static void
pass ()
{
	(void) fprintf (back, BI_AUTH "\n");
	exit (SUCCESS);
}

int
main(int argc, char *argv[], char *envp[])
{
	int i;

	/* Option string and current option character. */
	char *options = "ds:v:";
	int optchar;

	/* Service name. */
	char *service = NULL;

	/* Number of real arguments. */
	int narg;

	/* Class and username. */
	char *variable = NULL;
	char *class = NULL;
	char *username = NULL;

	/* Status code. */
	int status;

	/* Set up signal handling. */
	(void) signal (SIGQUIT, SIG_IGN);
	(void) signal (SIGINT, SIG_IGN);

	/* Set priority to default. */
	(void) setpriority (PRIO_PROCESS, 0, 0);

	/* Open syslog. */
	openlog (ident, LOG_ODELAY, LOG_AUTH);

	/* Echo arguments, switches and environment as they are. */
	for (i=0; i<argc; i++)
	{
		log ("Argument %d is '%s'", i, argv[i]);
	}
	for (i=0; envp[i]!=NULL; i++)
	{
		log ("Environment %d is %s", i, envp[i]);
	}

	/* Deal with the options. */
	opterr = 0;
	optchar = getopt (argc, argv, options);
	while (optchar != -1)
	{
		switch (optchar) {
		case 'd':

			/* Debug. */
			back = stdout;
			log ("Debug on");
			break;
		case 's':

			/* Service name. */
			service = optarg;
			log ("Service is %s", service);
			break;
		case 'v':

			/* Variable. */
			variable = optarg;
			log ("Variable is %s", variable);
			break;
		default:
			error ("usage error");
			break;
		}
		optchar = getopt (argc, argv, options);
	}

	/* Real arguments. */
	narg = argc - optind;
	if (narg > 0)
	{
		username = argv[optind];
	}
	if (narg > 1)
	{
		class = argv[optind + 1];
	}
	if (narg > 2)
	{
		error ("usage error");
	}
	if (username != NULL)
	{
		log ("Username is %s", username);
	}
	if (class != NULL)
	{
		log ("Class is %s", class);
	}

	/* Pledge. */
	status = pledge ("stdio rpath tty id", NULL);
	if (status < 0)
	{
		error ("pledge: %m");
	}

	/* Reopen back channel. */
	back = fdopen (3, "r+");
	if (back == NULL)
	{
		error ("reopening back channel: %m");
	}

	/* Fail out. */
	log ("User %s failed out", username);
	fail ("Fail");

	/* Finish. */
	closelog ();
}

/* End of file LOGIN_ECHO.C. */

