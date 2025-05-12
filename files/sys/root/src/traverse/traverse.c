
/* Traverse directory tree. */

/* Includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <pwd.h>
#include <err.h>
#include <util.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

/* Boolean. */
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* Status. */
#define SUCCESS 0
#define FAILURE 1

/* End of string character. */
#define EOS '\0'

/* Error exit. */

static void
error (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vprintf (format, args);
	(void) printf ("\n");
	va_end (args);

	/* Quit. */
	exit (FAILURE);
}

/* Function. */

static void
fix (char *path)
{
	int status;
	struct stat sb;
	uid_t u;
	gid_t g;
	mode_t m;

	status = stat (path, &sb);
	if (status < 0)
	{
		error ("Error calling stat");
	}
	u = sb.st_uid;
	g = sb.st_gid;
	m = sb.st_mode;
	(void) printf ("%06o %6d %6d %s\n", (int) m, (int) u, (int) g, path);
}

/* Traverse directory. */

static void
traverse (char *name,
	void (*f) (char *))
{
	DIR *dirp;
	struct dirent *dirent;
	char d[MAXNAMLEN + 1];
	char *dir;
	static int recurse = false;

	if (! recurse)
	{
		f (name);
	}
	dirp = opendir (name);
	dirent = readdir (dirp);
	while (dirent != NULL)
	{
		dir = dirent->d_name;
		if (! (strcmp (dir, ".") == 0 ||
			strcmp (dir, "..") == 0))
		{
			*d = '\0';
			(void) strcat (d, name);
			(void) strcat (d, "/");
			(void) strcat (d, dir);
			f (d);
			if (dirent->d_type == DT_DIR)
			{

				/* Directory. */
				recurse = true;
				traverse (d, f);
			}
			else if (dirent->d_type == DT_REG)
			{

				/* File. */
				;
			}
		}
		dirent = readdir (dirp);
	}
	closedir(dirp);
}

/* Print help. */

static void
usage (void)
{
	(void) printf ("Usage: traverse -h directory\n");
	exit (FAILURE);
}

/* Main. */

int
main(int argc, char **argv)
{
	char *optstr = "h";
	int ch;
	char *dir;

	/* Options. */
	opterr = 0;
	ch = getopt (argc, argv, optstr);
	while (ch != -1)
	{
		switch (ch)
		{
		case 'h':
			usage ();
			break;
		default:
			usage();
			break;
		}
		ch = getopt (argc, argv, optstr);
	}

	/* Traverse. */
	dir = argv[optind];
	if (dir == NULL)
	{
		error ("Specify directory");
	}
	traverse (dir, fix);

	/* Finish. */
	exit (SUCCESS);
}

