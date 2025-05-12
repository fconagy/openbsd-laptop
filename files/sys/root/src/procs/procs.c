
/* PROCS.C print process info. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <paths.h>
#include <kvm.h>
#include <sys/sysctl.h>
#include <sys/user.h>

/* Boolean constants, in case we don't have. */
#ifndef true
#define true ((int) 1)
#endif
#ifndef false
#define false ((int) 0)
#endif

#define SUCCESS 0
#define FAILURE 1

/* Error exit. */

static void
error (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vfprintf (stderr, format, args);
	va_end (args);
	(void) fprintf (stderr, "\n");
	(void) fflush (stderr);
	exit (FAILURE);
}

/* Print message. */

static void
msg (const char *format, ...)
{
    va_list args;

	/* Print message. */
	va_start (args, format);
	(void) vfprintf (stdout, format, args);
	va_end (args);
	(void) fprintf (stdout, "\n");
}

static void
cleanup (kvm_t *kd)
{
	int status;

	status = kvm_close (kd);
	if (status < 0)
	{
		error ("kvm_close failed with error %d\n", errno);
	}
}

/* Print info about a process. */

static void
printproc (int join, struct kinfo_proc *proc)
{
	if (join)
	{
		(void) printf ("%8u %8u %s",
			proc->p_pid,
			proc->p_uid,
			proc->p_comm);
	}
	else
	{
		msg ("%8u %8u %s",
			proc->p_pid,
			proc->p_uid,
			proc->p_comm);
	}
}

/* Print argv values for the process. */

static void
printargs (int join, kvm_t *kd, struct kinfo_proc *proc)
{
	char **procargs;
	char *arg;
	int i;

	/* This does not work for kernel threads. */
	procargs = kvm_getargv (kd, proc, 0);
	if (procargs == NULL)
	{
		cleanup (kd);
		error ("Function kvm_getargv failed");
	}
	i = 0;
	arg = procargs[i];
	while (arg != NULL)
	{
		if (join)
		{
			(void) printf (" %s", arg);
		}
		else
		{
			msg ("%17s %s", "", arg);
		}
		i++;
		arg = procargs[i];
	}
	if (join)
	{
		(void) printf ("\n");
	}
}

/* Print help. */

static void
printhelp (void)
{
	(void) fprintf (stdout, "\
This program show process info.\n\
Usage\n\
    procs [-k][-h]\n\
where\n\
    -k              show also kernel processes.\n\
    -h              print this help.\n\
");
	exit (FAILURE);
}

/* Main program, shows processes. */

int
main (int argc, char *argv[])
{

	/* Options. */
	char *options = "jkh";
	char opt;
	int kernel = false;
	int join = false;

	/* Error buffer to be used with kvm functions. */
	char errmsg[_POSIX2_LINE_MAX];

	/* Descriptors used by kvm functions. */
	kvm_t *kd;
	struct kinfo_proc *procs;
	struct kinfo_proc *proc;

	/* Number of processes returned. */
	int nprocs;

	/* Arguments passed to the process. */
	char **procargs;
	char *arg;

	/* Counter. */
	int i;
	int j;

	/* Deal with the options. */
	opterr = 0;
	opt = getopt (argc, argv, options);
	while (opt != (char) -1)
	{
		switch (opt)
		{
		case 'j':
			join = true;
			break;
		case 'k':
			kernel = true;
			break;
		case 'h':
			printhelp ();
			break;
		case '?':
		default:
			error ("Wrong option");
			break;
		}
		opt = getopt (argc, argv, options);
	}

	/* Get kvm descriptor. */
	kd = kvm_openfiles (NULL, NULL, NULL, KVM_NO_FILES, errmsg);
	if (kd == NULL)
	{
		error ("Function kvm_openfiles failed with %s", errmsg);
	}

	/* Get process info. */
	if (kernel)
	{
		procs = kvm_getprocs (kd, KERN_PROC_KTHREAD, 0,
			sizeof (struct kinfo_proc), &nprocs);
	}
	else
	{
		procs = kvm_getprocs (kd, KERN_PROC_ALL, 0,
			sizeof (struct kinfo_proc), &nprocs);
	}
	if (procs == NULL)
	{
		cleanup (kd);
		error ("Function kvm_getprocs failed");
	}

	/* Print what we got. */
	for (i = 0; i<nprocs; ++i)
	{
		proc = &procs[i];
		printproc (join, proc);
		if (! kernel)
		{
			printargs (join, kd, proc);
		}
	}

	/* Finish. */
	cleanup (kd);
	exit (SUCCESS);
}

/* End of file PROCS.C. */

