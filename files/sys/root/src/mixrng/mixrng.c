
/* MIXRNG.C is a pseudo device. */

/* Include files. */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <sys/conf.h>
#include <sys/ioctl.h>
#include <sys/ioccom.h>
#include <sys/malloc.h>
#include <sys/syslog.h>
#include <sys/time.h>

/* Important for vprintf.
   Otherwise there will be confusion. */
#include <sys/stdarg.h>

/* Our include file. */
#include "mixrngio.h"

/* Our name. */
#define DRIVERNAME "mixrng"

/* Our name. */
char *name = DRIVERNAME;

/* Debug preprocessor control flags. */

/* Uncomment this to enable debug. Compile time.
#define MIXRNG_DEBUG 2
*/

/* !!!! Testing, disables basic mixing functionality
   when defined.
#define MIXRNG_TURNOFF 1
*/

/* !!!! Testing, allow reads when defined. Should be
   off for production.
#define MIXRNG_ALLOWREAD 1
*/

/* Global debug switch. */
#ifdef MIXRNG_DEBUG
static int debug = MIXRNG_DEBUG;
#else
static int debug = 0;
#endif

/* Log line size. */
#define LOG_SIZE 256

/* Buffer size. Fit for a slower device. */
#define BUFFER_SIZE 512

/* Buffer. */
static u_char *buffer = NULL;

/* Nothing but the truth. */
#ifndef true
#define true ((int) 1)
#endif
#ifndef false
#define false ((int) 0)
#endif

/* Min for size_t. */

static size_t
stmin (size_t a, size_t b)
{
	return (a < b ? a : b);
}

/* Debug printf. */

static void
dpf (const char *format, ...)
{
/* No variadic macros so it is a real function call. */
#ifdef MIXRNG_DEBUG
	va_list args;
	char m[LOG_SIZE];

	if (debug > 0)
	{

		/* Using syslog to avoid console messages on the
		   VGA screen. */
		(void) memset (m, 0, (size_t) LOG_SIZE);
		va_start (args, format);
		(void) vsnprintf (m, (size_t) LOG_SIZE, format, args);
		va_end (args);
		log (LOG_DEBUG, "%s: %s\n", name, m);
	}
#endif
}

/* Log message. */

static void
msg (const char *format, ...)
{
	va_list args;
	static char m[LOG_SIZE];

	/* Don't pass format string ending with new line since
	   appearantly it will come out as printed twice in the log? */
	(void) memset (m, 0, (size_t) LOG_SIZE);
	va_start (args, format);
	(void) vsnprintf (m, (size_t) LOG_SIZE, format, args);
	va_end (args);
	log (LOG_INFO, "%s: %s\n", name, m);
}

/* Lock functions. Not used.
   static struct mutex lock;
   mtx_enter
   mtx_leave
   msleep_nsec */

/* Driver callback entry points. */
void mixrngattach (int num);
int mixrngopen (dev_t dev, int flag, int mode, struct proc *p);
int mixrngclose (dev_t dev, int flag, int mode, struct proc *p);
int mixrngread (dev_t dev, struct uio *uio, int flags);
int mixrngwrite (dev_t dev, struct uio *uio, int flags);
int mixrngioctl (dev_t dev, u_long cmd, caddr_t addr,
	int flag, struct proc *p);

/* Global variables. */

/* Attach done. Or not. */
static int attached = false;
static int broken = false;

/* File open flag. */
static int opened = false;

/* Start time and current time. */
static time_t opentime = 0;

/* Number of buffer I/Os done since last open. */
static long int nr = 0;
static long int nw = 0;

/* Number of buffers read/written since boot. */
static long int totr = 0;
static long int totw = 0;

/* Bytes read/written since last open. */
static long int byter = 0;
static long int bytew = 0;

/* Bytes read/written since boot. */
static long int totbyter = 0;
static long int totbytew = 0;

/* Print statistics. */

static void
printstat (void)
{
	time_t now;
	long int elaopen;
	long int elastart;
	long int scale = 1024;

	/* Elapsed time since open and boot. */
	now = gettime ();
	elaopen = (long int) (now - opentime);
	elastart = (long int) (getuptime());

	/* Log stat. */
	msg ("%44s: %24ld", "Number of records read since last open", nr);
	msg ("%44s: %24ld", "Number of records written since last open", nw);
	msg ("%44s: %24ld", "Number of bytes read since last open", byter);
	msg ("%44s: %24ld", "Number of bytes written since last open", bytew);
	msg ("%44s: %24ld", "Number of records read since boot", totr);
	msg ("%44s: %24ld", "Number of records written since boot", totw);
	msg ("%44s: %24ld", "Number of bytes read since boot", totbyter);
	msg ("%44s: %24ld", "Number of bytes written since boot", totbytew);
	if (elaopen == (time_t) 0)
	{
		msg ("%44s: %24s KiB/s", "Read bandwidth averaged since last open",
			"n/a");
		msg ("%44s: %24s KiB/s", "Write bandwidth averaged since last open",
			"n/a");
	}
	else
	{
		msg ("%44s: %24ld KiB/s", "Read bandwidth averaged since last open",
			byter / scale / elaopen);
		msg ("%44s: %24ld KiB/s", "Write bandwidth averaged since last open",
			bytew / scale / elaopen);
	}
	if (elastart == (time_t) 0)
	{
		msg ("%44s: %24s KiB/s", "Read bandwidth averaged since boot",
			"n/a");
		msg ("%44s: %24s KiB/s", "Write bandwidth averaged since boot",
			"n/a");
	}
	else
	{
		msg ("%44s: %24ld KiB/s", "Read bandwidth averaged since boot",
			totbyter / scale / elastart);
		msg ("%44s: %24ld KiB/s", "Write bandwidth averaged since boot",
			totbytew / scale / elastart);
	}
	msg ("%44s: %24ld", "Up seconds", elastart);
	msg ("%44s: %24ld", "Total bytes", totbyter + totbytew);
}

/* Print integer as hex. */

static void
printint (int a)
{
	int b;
	unsigned char *c;
	int i;
	int len;

	if (debug > 1)
	{
		b = a;
		c = (unsigned char *) &b;
		len = sizeof (int);
		for (i=0; i<len; i++)
		{
			addlog ("%02x", c[i]);
		}
		addlog (" ");
	}
}

/* Print new line. */

static void
printnl ()
{
	if (debug > 1)
	{
		addlog ("\n");
	}
}

/* Print buffer as hex bytes. */

static void
printbuf (void *buf, size_t len)
{
	unsigned char *charbuf;
	long int i;
	unsigned char ch;

	if (debug > 1)
	{
		log (LOG_DEBUG, "mixrng: buffer: ");
		charbuf = (unsigned char *) buf;
		for (i=0; i<(long int) len; i++)
		{
			ch = charbuf[i];
			addlog ("%02x", ch);
		}
		addlog ("\n");
	}
}

/* Send buffer to the random pool. */

static int
sendpool (u_char *buf, size_t len)
{
	size_t chunk;
	size_t count;
	size_t remainder;
	int *intbuf;
	long int i;
	long int nint;
	int rnd;

	/* Function enqueue_randomness requires int. */
	chunk = sizeof (int);
	count = len / chunk;
	remainder = len - (count * chunk);

	/* Buffer size should be a multiple of chunk size. */
	if (remainder != (size_t) 0)
	{
		dpf ("there is a remainder in sendpool");
		return (EINVAL);
	}

	intbuf = (int *) buf;
	nint = (long int) count;
	dpf ("sending %ld bytes", nint);
	for (i=0; i<nint; i++)
	{
		rnd = intbuf[i];
		printint (rnd);
/* !!!! Testing, turn of adding randomness to the pool. */
#ifndef MIXRNG_TURNOFF

		/* Send randomness to the pool. */
		enqueue_randomness (rnd);
#endif
	}
	printnl ();
	return (0);
}

/* Attach. */

void
mixrngattach (int num)
{

	/* Check. */
	if (num != 1)
	{

		/* There has to be only one. */
		dpf ("wrong number of devices");
		attached = false;
		broken = true;
	}
	else
	{
		attached = true;
		broken = false;
	}
	/* Note that gettime at this early stage of boot returns
	   time elapsed since boot appearantly. Time seems to be
	   not set yet. */
	(void) printf ("mixrng: attach\n");
}

/* Open. */

int
mixrngopen (dev_t dev, int flag, int mode, struct proc *p)
{
	size_t buffersize;

	/* Check. */
	if (!attached)
	{
		dpf ("device not attached");
		return (ENXIO);
	}
	if (broken)
	{
		dpf ("device broken");
		return (ENXIO);
	}
	if (opened)
	{
		dpf ("second open");
		return (EINVAL);
	}

/* Read has no role in the finals. */
#ifndef MIXRNG_ALLOWREAD
	if (flag & FREAD)
	{
		return (EPERM);
	}
#endif

	/* Buffer size. */
	buffersize = (size_t) BUFFER_SIZE;

	/* Allocate I/O buffer. */
	buffer = malloc (buffersize, M_TEMP, M_WAITOK);
	if (buffer == NULL)
	{
		return (ENOMEM);
	}
	(void) memset (buffer, 0, buffersize);

	/* Counters. */
	nr = 0;
	nw = 0;
	byter = 0;
	bytew = 0;
	opentime = gettime ();

	/* Show what we got. */
	dpf ("opened");
	dpf ("device number %016x major %d minor %d",
		dev, major (dev), minor (dev));
	dpf ("open flag %0x mode %0x", flag, mode);
	opened = true;
	return (0);
}

/* Close. */

int
mixrngclose (dev_t dev, int flag, int mode, struct proc *p)
{
	size_t buffersize;

	buffersize = (size_t) BUFFER_SIZE;
	(void) memset (buffer, 0, buffersize);
	free (buffer, M_TEMP, buffersize);
	opened = false;
	dpf ("closed");
	return (0);
}

/* Read. */

int
mixrngread (dev_t dev, struct uio *uio, int flags)
{
	u_char *buf;
	size_t bufsize;
	size_t count;
	int status;

	/* Initialize. */
	buf = buffer;
	bufsize = (size_t) BUFFER_SIZE;

	/* Check. */
	if (!attached)
	{
		dpf ("read but not attached");
		return (ENXIO);
	}
	if (uio->uio_iovcnt != 1)
	{
		dpf ("cannot do scatter/gather");
		return (ENXIO);
	}
	if (uio->uio_offset < (off_t) 0)
	{
		dpf ("read wrong offset");
		return (EINVAL);
	}
	if (uio->uio_rw == UIO_WRITE)
	{
		dpf ("read but uio_rw flag shows write");
		return (EINVAL);
	}
	if (!opened)
	{
		dpf ("read but not open");
		return (ENXIO);
	}

/* Read has no function in the final version. */
#ifndef MIXRNG_ALLOWREAD
	return (EPERM);
#endif

	/* Done already. */
	if (uio->uio_resid == (size_t) 0)
	{

		/* Finished already. */
		dpf ("read returning empty");
		return (0);
	}

	/* Start work. */
	while (uio->uio_resid > 0)
	{

		/* Either full buffer or short read. */
		count = stmin (bufsize, uio->uio_resid);

		/* Show the buffer. */
		dpf ("record %ld read %lu bytes at offset %lld",
			nr, count, uio->uio_offset);

/* !!!! Known buffer content, testing. */
#ifdef MIXRNG_TURNOFF
		(void) memset (buf, (int) nr, count);
#endif

		/* Show what we get if debug. */
		printbuf (buf, count);

		/* Read. Move buffer to user. */
		status = uiomove (buf, count, uio);
		if (status != 0)
		{
			dpf ("read uiomove failed status %d", status);
			return (status);
		}
		nr++;
		totr++;
		byter += count;
		totbyter += count;

		/* Give chance to others. */
		if (uio->uio_resid > 0)
		{
			yield ();
		}
	}

	/* Finish. */
	return (0);
}

/* Write. */

int
mixrngwrite (dev_t dev, struct uio *uio, int flags)
{
	u_char *buf;
	size_t bufsize;
	size_t count;
	int status;

	/* Init variables. */
	buf = buffer;
	bufsize = (size_t) BUFFER_SIZE;

	/* Check. */
	if (!attached)
	{
		dpf ("write but not attached");
		return (ENXIO);
	}
	if (uio->uio_iovcnt != 1)
	{
		dpf ("cannot do scatter/gather");
		return (ENXIO);
	}
	if (uio->uio_offset < (off_t) 0)
	{
		dpf ("write wrong offset");
		return (EINVAL);
	}
	if (uio->uio_rw == UIO_READ)
	{
		dpf ("write but uio_rw flag shows read");
		return (EINVAL);
	}
	if (!opened)
	{
		dpf ("write but not open");
		return (ENXIO);
	}

	/* !!!! This was count == (size_t) 0 ? */
	/* Done already. */
	if (uio->uio_resid == (size_t) 0)
	{

		/* Finished already. */
		dpf ("write returning empty");
		return (0);
	}

	/* Start work. */
	while (uio->uio_resid > 0)
	{

		/* Either full buffer or short read. */
		count = stmin (bufsize, uio->uio_resid);

		/* Write. Get buffer from user. */
		dpf ("record %ld read %lu bytes at offset %lld",
			nw, count, uio->uio_offset);
		status = uiomove (buf, count, uio);
		if (status != 0)
		{
			dpf ("write uiomove failed status %d", status);
			return (status);
		}

		/* Send it to the pool. */
		sendpool (buf, count);

		/* Show the buffer. */
		printbuf (buf, count);

		/* Update statistics. */
		nw++;
		totw++;
		bytew += count;
		totbytew += count;

		/* Give chance to others. */
		if (uio->uio_resid > 0)
		{
			yield ();
		}
	}

	/* Finish. */
	return (0);
}

/* Do ioctl. */

int
mixrngioctl (dev_t dev, u_long cmd, caddr_t addr,
	int flag, struct proc *p)
{
	struct mixrng_params *params;

	params = (struct mixrng_params *) addr;
	switch (cmd)
	{
		case MIXRNGIO_TEST:

			/* Test ioctl. */
			dpf ("received ioctl code %d data '%s'",
			       params->code, params->data);
			break;
		case MIXRNGIO_DEBUG:

			/* Set debug level. */
			dpf ("received ioctl set debug level %d",
			       params->code);
			debug = params->code;
			break;
		case MIXRNGIO_STAT:

			/* Log stat. */
			printstat ();
			break;
		default:
			dpf ("invalid ioctl command code");
			return (EINVAL);
			break;
	}
	return (0);
}

/* End of file MIXRNG.C. */

