
/* QUSB.C Quantis USB minimal library for Unix systems. */

/* Based on the Quantis utilities source, see copyright there. */

/* In this case we presume that there is only one Quantis USB device
   connected. */

/* System include files. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <signal.h>
#include <errno.h>

/* LIBUSB include. Package should be installed.
   Under OpenBSD the library is under /usr/local/lib and
   the header files in /usr/local/include . */
#include <libusb-1.0/libusb.h>

/* Our include file. */
#include "qusb.h"

/* Globals. */

/* Debug flag compiled in. */
#ifdef DEBUG
static int debug = DEBUG;
#else
static int debug = 0;
#endif

/* Global variable used by the signal handler.
   This is the Quantis device to clean up.
   We have only one device since it is USB. */
static qdev_t *quantis = NULL;

/* Message. */

static void
msg (const char *format, ...)
{
    va_list args;

	va_start (args, format);
	(void) vfprintf (stdout, format, args);
	va_end (args);
	(void) fprintf (stdout, "\n");
}

/* Debug message. */

static void
dmsg (const char *format, ...)
{
    va_list args;

	if (debug > 0)
	{
		(void) fprintf (stdout, "==== ");
		va_start (args, format);
		(void) vfprintf (stdout, format, args);
		va_end (args);
		(void) fprintf (stdout, "\n");
	}
}

/* Error message. */

static void
emsg (const char *format, ...)
{
    va_list args;

	if (debug > 0)
	{
		va_start (args, format);
		(void) vfprintf (stderr, format, args);
		va_end (args);
		(void) fprintf (stderr, "\n");
	}
}

/* Report Quantis error. */

extern void
qemsg (int code, const char *format, ...)
{
	int qc;
    va_list args;
	const char *errortext;

	if (debug > 0)
	{

		/* Code is a Quantis error code, which is 0 .. -9 and -99.
		   Zero is actually success. */
		qc = abs (code);
		if (0 <= qc && qc <= 9)
		{
			errortext = qerrormessages[qc];
		}
		else
		{
			errortext = qerrormessages[10];
		}
		(void) fprintf (stderr, "QUSB library error code %d %s\n",
			code, errortext);

		/* The rest. */
		va_start (args, format);
		(void) vfprintf (stderr, format, args);
		va_end (args);
		(void) fprintf (stderr, "\n");
	}
}

/* Report LIBUSB error. */

extern void
uemsg (int code, const char *format, ...)
{
    va_list args;
	const char *errortext;

	if (debug > 0)
	{

		/* LIBUSB error. */
		errortext = libusb_error_name (code);
		(void) fprintf (stderr, "LIBUSB error code %d %s\n",
			code, errortext);

		/* The rest. */
		va_start (args, format);
		(void) vfprintf (stderr, format, args);
		va_end (args);
		(void) fprintf (stderr, "\n");
	}
}

/* Error exit. */

static void
error (const char *format, ...)
{
    va_list args;

	va_start (args, format);
	(void) vfprintf (stderr, format, args);
	va_end (args);
	(void) fprintf (stderr, "\n");
	(void) fflush (stderr);
	exit (FAILURE);
}

/* Compare strings. */
static int
seq (char *s1, char *s2)
{
	return (strcmp (s1, s2) == 0);
}

/* Set debug level. */

extern void
qdebug (int level)
{
	debug = level;
}

/* Print buffer. */

extern void
qprintbuffer (char *buffer, int size, char *delim,
	int format, int linelength)
{
	int wordlength = sizeof (word);
	int l;
	int w;
	int i;
	int nwords;
	int nbytes;
	int npos;
	unsigned char ch;

	l = 0;
	w = 0;
	for (i=0; i<size; i++)
	{
		if (w == 0 && l != 0)
		{
				(void) fprintf (stdout, "%s", delim);
		}
		ch = (unsigned char) buffer[i];
		switch (format)
		{
		case 0:

			/* Lowercase. */
			(void) fprintf (stdout, "%02x", ch);
			break;
		case 1:

			/* Uppercase. */
			(void) fprintf (stdout, "%02X", ch);
			break;
		default:

			/* Default is lowercase. */
			(void) fprintf (stdout, "%02x", ch);
			break;
		}
		nbytes = w + 1;
		if (nbytes >= wordlength)
		{
			nwords = l + 1 + 1;
			npos = nwords * (wordlength * 2) + (nwords - 1);
			if (npos >= linelength)
			{
				(void) fprintf (stdout, "\n");
				l = 0;
			}
			else
			{
				l++;
			}
			w = 0;
		}
		else
		{
			w++;
		}
	}

	/* Final new line. */
	if (l != 0)
	{
		(void) fprintf (stdout, "\n");
	}
	else if (w != 0)
	{
				(void) fprintf (stdout, "\n");
	}
}

/* Create Quantis descriptor. */

extern qdev_t *
qcreate (void)
{
	qdev_t *r;

	r = (qdev_t *) malloc (sizeof (qdev_t));
	if (r == NULL)
	{
		return (NULL);
	}
	r->claimed = false;
	r->opened = false;
	r->context = NULL;
	r->devices = NULL;
	r->dev = NULL;
	r->handle = NULL;
	r->desc = (struct libusb_device_descriptor *)
		malloc (sizeof (struct libusb_device_descriptor));
	if (r->desc == NULL)
	{
		return (NULL);
	}
	r->usbconfig = NULL;
	r->serial = (char *) malloc (STRINGSIZE + 1);
	if (r->serial == NULL)
	{
		return (NULL);
	}
	r->manufacturer = (char *) malloc (STRINGSIZE + 1);
	if (r->manufacturer == NULL)
	{
		return (NULL);
	}
	r->product = (char *) malloc (STRINGSIZE + 1);
	if (r->product == NULL)
	{
		return (NULL);
	}
	r->version = 0;
	r->datarate = 0;
	r->busnumber = 0;
	r->deviceaddress = 0;
	r->pos = 0;
	r->packetsize = 0;
	r->endpoint = 0;
	r->packet = NULL;
	return (r);
}

/* Deallocate descriptor. */

static void
qfree (qdev_t *q)
{
	dmsg ("Free qdev");
	(void) memset (q->desc, 0, sizeof (struct libusb_device_descriptor));
	free (q->desc);
	(void) memset (q->serial, 0, (size_t) STRINGSIZE + 1);
	free (q->serial);
	(void) memset (q->manufacturer, 0, (size_t) STRINGSIZE + 1);
	free (q->manufacturer);
	(void) memset (q->product, 0, (size_t) STRINGSIZE + 1);
	free (q->product);
	(void) memset (q, 0, sizeof (qdev_t));
	free (q);
	dmsg ("We are all free now");
}

/* Cleanup and close. */

static void
qclean (qdev_t *q)
{
	dmsg ("Cleaning up");
	if (q == NULL)
	{
		return;
	}
	if (q->claimed)
	{
		libusb_release_interface (q->handle, 0);
		q->claimed = false;
	}
	if (q->opened)
	{

		/* This explodes giving an abort trap.
		   It happens undeterministcally, sometimes
		   it works with debug enabled, sometimes just
		   explodes just the same.
		   Appearantly there is no way to cancel
		   transfers when using synchronous protocol.
		   So this is disabled at this point.
		   Seems to be working, enabled back.
		*/

		dmsg ("LIBUSB close");
		libusb_close (q->handle);
		q->opened = false;
		q->handle = NULL;
	}
	if (q->usbconfig != NULL)
	{
		libusb_free_config_descriptor (q->usbconfig);
		q->usbconfig = NULL;
	}
	if (q->devices != NULL)
	{
		libusb_free_device_list (q->devices, 1);
		q->devices = NULL;
		q->dev = NULL;
	}
	if (q->context != NULL)
	{
		libusb_exit (q->context);
		q->context = NULL;
	}
	q->version = 0;
	q->datarate = 0;
	if (q->packet != NULL)
	{
		(void) memset (q->packet, 0, q->packetsize);
		free (q->packet);
		q->packet = NULL;
	}
	dmsg ("Cleaning up finished, deallocating q_dev");
	qfree (q);
	q = NULL;
}

/* Destroy descriptor. Will also close if forgotten. */

extern void
qdestroy (qdev_t *q)
{
	qclean (q);
}

/* Cleanup before reporting error in enumerate. */

static void
free_device_exit (qdev_t *q, libusb_device **devices, libusb_context *context)
{
	libusb_free_device_list (devices, 1);
	q->devices = NULL;
	libusb_exit (context);
	q->context = NULL;
}

/* Count set bits. */

static int
countsetbits (int value)
{
	size_t i;
	int count;

	count = 0;
	for (i = 0; i < (sizeof (value) * 8); i++)
	{
		if (value & (1 << i))
		{
			count++;
		}
	}
	return (count);
}

/* Get vendor integer value. */

static int
qgetint (qdev_t *q, char request)
{
	uint8_t rtype;
	uint16_t value = 0;
	uint16_t index = 0;
	unsigned char buffer[4];
	int status;

	rtype = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE |
		LIBUSB_ENDPOINT_IN;
	status = libusb_control_transfer (q->handle, rtype, request, value, index,
		(unsigned char *) buffer, sizeof (buffer), QUANTIS_USB_REQUEST_TIMEOUT);
	if(status < 0)
	{
		uemsg (status, "Error calling control transfer");
		return (QUANTIS_ERROR_IO);
	}

	status = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
	return (status);
}

/* Get vendor unsigned char value. */

static int
qgetuchar (qdev_t *q, char request)
{
	uint8_t rtype;
	uint16_t value = 0;
	uint16_t index = 0;
	unsigned char buffer[1];
	int status;

	rtype = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE |
		LIBUSB_ENDPOINT_IN;
	status = libusb_control_transfer (q->handle, rtype, request, value, index,
		(unsigned char *) buffer, sizeof (buffer), QUANTIS_USB_REQUEST_TIMEOUT);
	if (status < 0)
	{
		uemsg (status, "Error calling control transfer");
		return (QUANTIS_ERROR_IO);
	}

	status = (int) buffer[0];
	return (status);
}

/* Send vendor request. */

static int
qsend (qdev_t *q, char request)
{
	int status;
	uint8_t rtype;
	uint16_t value = 0;
	uint16_t index = 0;

	rtype = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE |
		LIBUSB_ENDPOINT_IN;
	status = libusb_control_transfer (q->handle, rtype, request, value, index,
		NULL, 0, QUANTIS_USB_REQUEST_TIMEOUT);
	if (status < 0)
	{
		uemsg (status, "Error calling control transfer");
		return (QUANTIS_ERROR_IO);
	}
	return (QUANTIS_SUCCESS);
}

/* Default signal handler. */

static void
qdefhand (int signum)
{
	dmsg ("Signal %d received - default handler running", signum);
	if (quantis != NULL)
	{
		dmsg ("Calling cleanup in signal handler");
		sleep (1);
		msg ("Exiting, cleaning up");
		qclean (quantis);
	}
	exit (FAILURE);
}

/* Macro to establish signal handler. */

#define sig(s, h) \
	if (signal (s, h) == SIG_ERR) \
	{ \
		dmsg ("Establishing signal handler for signal %d failed with %d", \
			s, errno); \
		return (QUANTIS_ERROR_INVALID_PARAMETER); \
	}

/* Establish signal handler. */

extern int
qsignal (qdev_t *q, void (*handler)(int))
{
	if (q == NULL)
	{
		dmsg ("Invalid qdev argument - null");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	if (handler == NULL)
	{
		dmsg ("Establishing our default signal handler");
		sig (SIGHUP, qdefhand);
		sig (SIGINT, qdefhand);
		sig (SIGQUIT, qdefhand);
	}
	else
	{
		dmsg ("Establishing custom signal handler");
		sig (SIGHUP, handler);
		sig (SIGINT, handler);
		sig (SIGQUIT, handler);
	}

	/* Set global variable. */
	quantis = q;
	return (QUANTIS_SUCCESS);
}

/* Put back default signal handler. */

extern int
qsignaldfl (qdev_t *q)
{
	if (q == NULL)
	{
		dmsg ("Invalid qdev argument - null");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	dmsg ("Establishing default signal handler");
	sig (SIGHUP, SIG_DFL);
	sig (SIGINT, SIG_DFL);
	sig (SIGQUIT, SIG_DFL);
	return (QUANTIS_SUCCESS);
}

/* Set LIBUSB debug level. */

extern void
qusbdebug (qdev_t *q, int level)
{
	libusb_context *context;

	/* Call libusb function to set debug level. */
	dmsg ("Libusb debug level set to %d", level);
	context = q->context;

	/* Enable libusb debug messages.
	   Possible log levels are:
	   _NONE, _ERROR, _WARNING, _INFO, _DEBUG */
	if (level < 3)
	{
		libusb_set_option (context, LIBUSB_OPTION_LOG_LEVEL,
			LIBUSB_LOG_LEVEL_NONE);
	}
	switch (level)
	{
	case 3:
		libusb_set_option (context, LIBUSB_OPTION_LOG_LEVEL,
			LIBUSB_LOG_LEVEL_ERROR);
		break;
	case 4:
		libusb_set_option (context, LIBUSB_OPTION_LOG_LEVEL,
			LIBUSB_LOG_LEVEL_WARNING);
		break;
	case 5:
		libusb_set_option (context, LIBUSB_OPTION_LOG_LEVEL,
			LIBUSB_LOG_LEVEL_DEBUG);
		break;
	}
	if (level > 5)
	{
		libusb_set_option (context, LIBUSB_OPTION_LOG_LEVEL,
			LIBUSB_LOG_LEVEL_DEBUG);
	}
}

/* Enumerate Quantis devices. */

extern int
qenumerate (qdev_t *q)
{
	int status = QUANTIS_ERROR_OTHER;
	libusb_context *context = NULL;
	libusb_device **devices = NULL;
	int ndev = 0;
	libusb_device *dev = NULL;
	struct libusb_device_descriptor *desc = NULL;
	int count = 0;
	int i = 0;
	int index = 0;

	/* Check. */
	if (q == NULL)
	{
		dmsg ("Qdev is null");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	if (sizeof (unsigned char) != 1)
	{

		/* Paranoid check. We cannot possibly cope with
		   this architecture. */
		dmsg ("Size of unsigned char is not 1 - confused");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}

	/* Descriptors for Quantis.
	   This is a local copy of the desc structure.
	   There is one retured per device as it strolls
	   through the device list. When the Quantis found
	   we copy out from this temporary structure to our
	   device descriptor. */
	desc = (struct libusb_device_descriptor *)
		malloc (sizeof (struct libusb_device_descriptor));
	if (desc == NULL)
	{
		dmsg ("No memory for libusb device descriptor");
		return (QUANTIS_ERROR_NO_MEMORY);
	}

	/* Initialize libusb. */
	dmsg ("Initializing");
	status = libusb_init (&(q->context));
	if (status < 0)
	{
		uemsg (status, "Initializing failed");
		libusb_exit (context);
		return (status);
	}
	context = q->context;

	/* Will return a list of USB devices currently attached to the system. */
	dmsg ("Get device list");
	ndev = (int) libusb_get_device_list (context, &(q->devices));
	if (ndev < 0)
	{
		uemsg (ndev, "Error getting device list");
		free_device_exit (q, devices, context);
		return (ndev);
	}
	devices = q->devices;

	/* Search for Quantis USB devices */
	for (i=0; i<ndev; i++)
	{
		dev = devices[i];
		if (dev == NULL)
		{
			dmsg ("Device pointer unexpectedly NULL");
			free_device_exit (q, devices, context);
			return (QUANTIS_ERROR_OTHER);
		}

		/* Get descriptor for the device and see if it is Quantis. */
		dmsg ("Get device descriptor");
		(void) memset (desc, 0, sizeof (struct libusb_device_descriptor));
		status = libusb_get_device_descriptor (dev, desc);
		if (status < 0)
		{
			uemsg (status, "Error getting device descriptor");
			free_device_exit (q, devices, context);
			return (status);
		}
    	if ((desc->idVendor == VENDOR_ID_ELLISYS) &&
			(desc->idProduct == DEVICE_ID_QUANTIS_USB))
		{

			/* Found Quantis. */
			count++;
			index = i;
			q->dev = dev;

			/* Copy out the device descriptor. */
			(void) memcpy (q->desc, desc, sizeof (struct libusb_device_descriptor));

			/* Get location. */
			q->busnumber = (int) libusb_get_bus_number (dev);
			q->deviceaddress = (int) libusb_get_device_address (dev);
			dmsg ("Found Quantis at index %d bus number %d device address %d",
				index, q->busnumber, q->deviceaddress);
		}
	}
	if (count == 0)
	{

			/* No device. */
			dmsg ("No device");
			free_device_exit (q, devices, context);
			return (QUANTIS_ERROR_NO_DEVICE);
	}
	if (count > 1)
	{

			/* More than one device, we are not prepared for that. */
			dmsg ("More than one device");
			free_device_exit (q, devices, context);
			return (QUANTIS_ERROR_INVALID_PARAMETER);
	}

	/* Done. */
	free (desc);
	desc = NULL;
	return (QUANTIS_SUCCESS);
}

/* Probe device if present. */

extern int
qprobe (void)
{
	qdev_t *q;
	int status;

	q = qcreate ();
	if (q == NULL)
	{
		return (false);
	}
	status = qenumerate (q);

	/* Not too fast. */
	(void) sleep (1);
	qdestroy (q);
	return (status == QUANTIS_SUCCESS);
}

/* Get board version. */

static int
qboardversion (qdev_t *q)
{
	return (qgetint (q, QUANTIS_USB_CMD_GET_BOARD_VERSION));
}

/* Get modules mask. */

static int
qgetmask (qdev_t *q)
{
	return (qgetuchar (q, QUANTIS_USB_CMD_GET_MODULES_MASK));
}

/* Get modules data rate. */

static int
qgetdatarate (qdev_t *q)
{
	int rate;
	int mask;

	rate = qgetint (q, QUANTIS_USB_CMD_GET_MODULES_RATE);
	if (rate < 0)
	{
		dmsg ("Bad rate");
		return (rate);
	}

	/* Modules rate is in kbit/s. Converting it to bytes per second. */
	rate = rate * 1000 / 8;

	mask = qgetmask (q);
	if (mask < 0)
	{
		dmsg ("Bad mask");
		return (mask);
	}
	return (rate * countsetbits (mask));
}

/* Get modules power. */
static int
qpower (qdev_t *q)
{
	return (qgetuchar (q, QUANTIS_USB_CMD_GET_MODULES_POWER));
}

/* Get module status. */

static int
qstatus (qdev_t *q)
{
  return (qgetuchar (q, QUANTIS_USB_CMD_GET_MODULES_STATUS));
}

/* Macro to check qdev. Should be not null and open. */

#define qcheck(q) \
	if (q == NULL) \
	{ \
		dmsg ("Qdev null"); \
		return (QUANTIS_ERROR_INVALID_PARAMETER); \
	} \
	if (! q->opened) \
	{ \
		dmsg ("Qdev not open"); \
		return (QUANTIS_ERROR_INVALID_PARAMETER); \
	}

/* Open device. */

extern int
qopen (qdev_t *q)
{
	int status = QUANTIS_ERROR_OTHER;
	libusb_device_handle *handle = NULL;
	libusb_device *dev = NULL;
	struct libusb_device_descriptor *desc = NULL;
	struct libusb_config_descriptor *usbconfig = NULL;
	struct libusb_interface *usbinterface = NULL;
	struct libusb_interface_descriptor
		*usbinterfacedescriptor = NULL;
	int packetsize;
	int endpoint;

	/* Check. */
	if (q == NULL)
	{
		dmsg ("Qdev NULL");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	if (q->opened)
	{

		/* Second open. */
		dmsg ("Second open");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}

	/* Initialize libusb and search for Quantis device. */
	dmsg ("Open/init device");
	status = qenumerate (q);
	if (status != QUANTIS_SUCCESS)
	{
		qemsg (status, "Enumerate failed");
		return (status);
	}
	dev = q->dev;
	desc = q->desc;

	/* LIBUSB open device. */
	status = libusb_open (dev, &(q->handle));
	if (status < 0)
	{
		uemsg (status, "USB open failed");
		qclean (q);
		return (status);
	}
	handle = q->handle;

	/* Set USBLIB debug level. */
	qusbdebug (q, debug);

	/* Set the active configuration for a device.
	   This also acts as a soft reset. If the device
	   does not support it that's also OK. */
	dmsg ("Set configuration");
	status = libusb_set_configuration (handle, 1);
	if (status < 0)
	{
		if (status != LIBUSB_ERROR_NOT_SUPPORTED)
		{
			uemsg (status, "Set configuration failed");
			qclean (q);
			return (status);
		}
	}

	/* Claim interface. */
	dmsg ("Claim interface");
	status = libusb_claim_interface (handle, 0);
	if (status < 0)
	{
		uemsg (status, "Claim interface failed");
		qclean (q);
		return (status);
	}
	q->claimed = true;

	/* Determine maximum packet size.
	   Here dev, config etc. are referred from a
	   previus enumeration. */

	/* Get config descriptor. */
	dmsg ("Get config descriptor");
	status = libusb_get_config_descriptor (dev, 0, &usbconfig);
	if (status < 0)
	{
		uemsg (status, "Get config descriptor failed");
		qclean (q);
		return (status);
	}
	q->usbconfig = usbconfig;

	/* Check. */
	if ((desc->bNumConfigurations != 1) || (usbconfig->bNumInterfaces != 1))
	{
		dmsg ("Wrong value in config descriptor");
		qclean (q);
		return (QUANTIS_ERROR_IO);
	}
	usbinterface = (struct libusb_interface *)
		usbconfig->interface;
	if (usbinterface[0].num_altsetting != 1)
	{
		dmsg ("Wrong value in config for interface");
		qclean (q);
		return (QUANTIS_ERROR_IO);
	}
	usbinterfacedescriptor = (struct libusb_interface_descriptor *)
		usbinterface[0].altsetting;
	if (usbinterfacedescriptor[0].bNumEndpoints != 1)
	{
		dmsg ("Wrong value in config for endpoints");
		qclean (q);
		return (QUANTIS_ERROR_IO);
	}

	/* Get the packetsize. */
	packetsize = (int) usbinterfacedescriptor[0].endpoint[0].wMaxPacketSize;
	q->packetsize = packetsize;
	dmsg ("Packet size found to be %d", packetsize);

	/* Get the endpoint.
	   Bits 0-3 is the endpoint number.
	   Bit 7 is direction. 1 is in, 0 is out.
	   Bits 4-6 reserved. */
	endpoint = (int) usbinterfacedescriptor[0].endpoint[0].bEndpointAddress;
	q->endpoint = endpoint;
	dmsg ("Endpoint number is %d", endpoint & 0x07);
	dmsg ("Endpoint direction is 0x%02x", endpoint & 0x1000);

	/* Force read from qbyte on first access.
	   The value for pos is 0 .. packetsize-1. */
	q->pos = packetsize;

	/* Allocate the packet buffer. */
	q->packet = (unsigned char *) malloc ((size_t) packetsize);
	if (q->packet == NULL)
	{
		qclean (q);
		return (QUANTIS_ERROR_NO_MEMORY);
	}
	(void) memset (q->packet, 0, (size_t) packetsize);

	/* Get the serial number. */
	status = libusb_get_string_descriptor_ascii (handle, desc->iSerialNumber,
				(unsigned char *) q->serial, STRINGSIZE);
	if (status < 0)
	{
		if (status != LIBUSB_ERROR_INVALID_PARAM)
		{
			uemsg (status, "Get string descriptor ascii failed for serial");
			qclean (q);
			return (status);
		}
		if (status >= STRINGSIZE)
		{
			qclean (q);
			dmsg ("String too big for serial number");
			return (QUANTIS_ERROR_INVALID_PARAMETER);
		}

		/* Unable to get serial. But that's OK. */
		(void) strncpy (q->serial, QUANTIS_NO_SERIAL, STRINGSIZE);
	}
	dmsg ("Serial is '%s' status %d", q->serial, status);

	/* Get manufacturer */
	status = libusb_get_string_descriptor_ascii (handle, desc->iManufacturer,
		(unsigned char *) q->manufacturer, STRINGSIZE);
	if (status < 0)
	{
		if (status != LIBUSB_ERROR_INVALID_PARAM)
		{
			uemsg (status, "Get string descriptor ascii failed");
			qclean (q);
			return (status);
		}
		if (status >= STRINGSIZE)
		{
			qclean (q);
			dmsg ("String too big for manufacturer");
			return (QUANTIS_ERROR_INVALID_PARAMETER);
		}

		/* Unable to get value. */
		(void) strncpy (q->manufacturer, QUANTIS_NOT_AVAILABLE, STRINGSIZE);
	}
	dmsg ("Manufacturer is '%s' status %d", q->manufacturer, status);

	/* Get product */
	status = libusb_get_string_descriptor_ascii (handle, desc->iProduct,
		(unsigned char *) q->product, STRINGSIZE);
	if (status < 0)
	{
		if (status != LIBUSB_ERROR_INVALID_PARAM)
		{
			uemsg (status, "Get string descriptor ascii failed");
			qclean (q);
			return (status);
		}
		if (status >= STRINGSIZE)
		{
			qclean (q);
			dmsg ("String too big for product");
			return (QUANTIS_ERROR_INVALID_PARAMETER);
		}

		/* Unable to get value. */
		(void) strncpy (q->product, QUANTIS_NOT_AVAILABLE, STRINGSIZE);
	}
	dmsg ("Product is '%s' status %d", q->product, status);

	/* Get board version. */
	q->version = qboardversion (q);

	/* Get data rate. */
	q->datarate = qgetdatarate (q);

	/* Finish. */
	q->opened = true;
	return (QUANTIS_SUCCESS);
}

/* Close. */

extern int
qclose (qdev_t *q)
{

	/* Check. */
	qcheck (q);

	/* Close. */
	dmsg ("Close");
	if (q->claimed)
	{
		libusb_release_interface (q->handle, 0);
		q->claimed = false;
	}
	if (q->opened)
	{
		libusb_close (q->handle);
		q->opened = false;
		q->handle = NULL;
	}
	return (QUANTIS_SUCCESS);
}

/* Read one packet with bulk transfer. */

extern int
qreadbulk (qdev_t *q)
{
	int status = QUANTIS_ERROR_OTHER;
	int transferred;
	int packetsize;

	/* Check if not null and open. */
	qcheck (q);

	/* Read data.
	   Note: We MUST request 'packetsize' amount of data
	   otherwise the request fails. */
	packetsize = q->packetsize;
	dmsg ("Bulk read %d bytes", packetsize);
   	status = libusb_bulk_transfer (q->handle, (unsigned char) QUANTIS_USB_ENDPOINT_BULK_IN,
		q->packet, packetsize, &transferred, (unsigned int) QUANTIS_USB_REQUEST_TIMEOUT);
	if (status != 0)
	{
		uemsg (status, "USB bulk transfer failed");
		return (status);
	}
   	if (transferred != packetsize)
   	{
		dmsg ("USB bulk transfer packet short");
		return (status);
   	}
	return (QUANTIS_SUCCESS);
}

/* Read next packet if the current one exhausted.
   Note that it refers variable 'status'. */

#define nextpacket(q) \
	if (q->pos >= q->packetsize) \
	{ \
		status = qreadbulk (q); \
		if (status < 0) \
		{ \
			qemsg (status, "Error reading bulk"); \
			return (status); \
		} \
		q->pos = 0; \
	}

/* Read one byte buffered. */

extern int
qbyte (qdev_t *q, byte *r)
{
	int status = QUANTIS_ERROR_OTHER;

	qcheck (q);
	nextpacket (q);
	*r = (byte) q->packet[q->pos];
	q->pos++;
	return (QUANTIS_SUCCESS);
}

/* Read one word buffered. */

extern int
qword (qdev_t *q, word *r)
{
	int status = QUANTIS_ERROR_OTHER;
	int i;
	word *p;

	qcheck (q);
	nextpacket (q);

	/* We take the address at the current position in the buffer. */
	p = (word *) (q->packet + q->pos);
	*r = *p;
	for (i=0; i<sizeof(word); i++)
	{
		q->pos++;
	}
	return (QUANTIS_SUCCESS);
}

/* Read one int buffered. */

extern int
qint (qdev_t *q, int *r)
{
	int status = QUANTIS_ERROR_OTHER;
	int i;
	int *p;

	qcheck (q);
	nextpacket (q);

	p = (int *) (q->packet + q->pos);
	*r = *p;
	for (i=0; i<sizeof(int); i++)
	{
		q->pos++;
	}
	return (QUANTIS_SUCCESS);
}

/* Read one word, unbuffered indirect, will read one packet for it. */

extern int 
qwordui (qdev_t *q, word *r)
{
	int status = QUANTIS_ERROR_OTHER;
	unsigned char *packet;
	word *ullpacket;
	int ullfactor;
	int packetsize;
	int max;
	int index;

	qcheck (q);
	status = qreadbulk (q);
	if (status < 0)
	{
		qemsg (status, "Error reading bulk");
		return (status);
	}
	packet = q->packet;
	ullpacket = (word *) packet;
	ullfactor = sizeof (word) / sizeof (unsigned char);
	packetsize = q->packetsize;
	max = packetsize / ullfactor;
	index = (int) (ullpacket[0] % max);
	dmsg ("%12d %12d", index, max);
	if (index >= max || index < 0)
	{
		dmsg ("Wrong index - confused");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	*r = ullpacket[index];
	return (QUANTIS_SUCCESS);
}

/* Read integer unbuffered indirect, will read one packet. */

extern int
qintui (qdev_t *q, int *r)
{
	int status = QUANTIS_ERROR_OTHER;
	word w;
	int result;

	status = qwordui (q, &w);
	if (status < 0)
	{
		qemsg (status, "Error running qwordui");
		return (status);
	}
	result = (int) (w & INT_MAX);
	*r = result;
	return (QUANTIS_SUCCESS);
}

/* Read scaled int. */

extern int
qscaledint (qdev_t *q, int indirect, int *r, int min, int max)
{
	int status = QUANTIS_ERROR_OTHER;
	int rnd;
	int range;
	int s;

	if (indirect)
	{

		/* Read scaled int unbuffered indirect. */
		status = qintui (q, &rnd);
	}
	else
	{
		status = qint (q, &rnd);
	}
	if (status < 0)
	{
		qemsg (status, "Error running qint");
		return (status);
	}

	/* For >=0 integers. */
	if (max <= min || max < 0 || min < 0)
	{
		dmsg ("Wong min/max");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	range = max - min + 1;
	s = (abs (rnd) % range) + min;
	*r = s;
	return (QUANTIS_SUCCESS);
}

/* Read scaled double. */

extern int
qscaleddouble (qdev_t *q, int indirect, double *r,
	double min, double max)
{
	int status = QUANTIS_ERROR_OTHER;
	double rnd;
	word w;
	double rnd01;
	double range;
	double s;

	/* Check. We strongly believe at this point that the word is unsigned long. */
#define WORD_MAX ULONG_MAX
	if (sizeof (word) != sizeof (double))
	{
		dmsg ("Size of word is not the same as size of double - confused");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	if (sizeof (word) != sizeof (unsigned long))
	{
		dmsg ("Size of word is not the same as size of unsigned long - confused");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	if (WORD_MAX != ULLONG_MAX)
	{
		dmsg ("Maximum unsigned long is not the same as unsigned long long - confused");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}

	/* More checks. For >= 0 real numbers. */
	if (max <= min || max < (double) 0.0 || min < (double) 0.0)
	{
		dmsg ("Max less than min or negative - confused");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	if (max > (double) WORD_MAX || min > (double) (WORD_MAX - 1))
	{
		dmsg ("Max or min too large - confused");
		return (QUANTIS_ERROR_INVALID_PARAMETER);
	}
	if (indirect)
	{

		/* Indirect unbuffered. */
		status = qwordui (q, &w);
	}
	else
	{
		status = qword (q, &w);
	}
	if (status < 0)
	{
		qemsg (status, "Error running qword");
		return (status);
	}
	rnd = (double) w;
	/* !!!! This was in the original Quantis code.
	   I don't understand this.
	rnd01 = rnd / ((double) 0xffffffffffffffffull + (double) 1.0);
	*/
	rnd01 = rnd / ((double) WORD_MAX);
	range = max - min;
	s = (rnd01 * range) + min;
	dmsg ("%12.10f\n", rnd);
	*r = s;
	return (QUANTIS_SUCCESS);
}

/* Read buffer. */

extern int
qread (qdev_t *q, void *buffer, size_t size)
{
	int buffersize;
	char *offset;
	char *packet;
	int packetsize;
	int npackets;
	int remainder;
	int i;
	int status = QUANTIS_ERROR_OTHER;

	/* Check. */
	qcheck (q);

	/* Read. */
	dmsg ("Read %llu bytes", size);

	/* Check if at least one module is present/enabled */
	if (qstatus (q) <= 0)
	{
		dmsg ("No module");
		return (QUANTIS_ERROR_NO_MODULE);
	}

	packet = q->packet;
	buffersize = (int) size;
	offset = (char *) buffer;
	packetsize = q->packetsize;
	npackets = buffersize / packetsize;
	remainder = buffersize - (npackets * packetsize);
	dmsg ("Will read %d bulk packets and %d remainder bytes", npackets, remainder);

	/* Read full packets. */
	for (i=0; i<npackets; i++)
	{

		/* Read bulk packet. */
		status = qreadbulk (q);
		if (status < 0)
		{
			return (QUANTIS_ERROR_IO);
		}

		/* Copy data to buffer */
		(void) memcpy (offset, packet, packetsize);
		offset += packetsize;
	}

	/* Copy the remainder, if any. */
	if (remainder > 0)
	{
		status = qreadbulk (q);
		if (status < 0)
		{
			return (QUANTIS_ERROR_IO);
		}
		(void) memcpy (offset, packet, remainder);
	}

	/* Finish. */
	return (QUANTIS_SUCCESS);
}

/* Report situation. */

extern void
qreport (qdev_t *q)
{
	if (q != NULL)
	{
		if (q->opened)
		{
			printf ("%40s %d\n", "Module power is", qpower (q));
			printf ("%40s %d\n", "Module status is", qstatus (q));
			printf ("%40s 0x%08X\n", "Board version is", q->version);
			printf ("%40s %d byte/s\n", "Data rate is", q->datarate);
			printf ("%40s %s\n", "Serial number is", q->serial);
			printf ("%40s %s\n", "Manufacturer is", q->manufacturer);
			printf ("%40s %d\n", "Bus number is", q->busnumber);
			printf ("%40s %d\n", "Device address is", q->deviceaddress);
			printf ("%40s 0x%08X\n", "Endpoint address is", q->endpoint);
			printf ("%40s %d\n", "Endpoint number is", q->endpoint & 0x07);
			printf ("%40s 0x%02x\n", "Endpoint direction is", q->endpoint & 0x10);
		}
	}
}

/* Return version. */

extern int
qmajor (void)
{
	return (MAJOR);
}

extern int
qminor (void)
{
	return (MINOR);
}

/* Print information. */

static void
info (void)
{
	msg ("%40s %3d byte", "char size is", sizeof (char));
	msg ("%40s %3d byte", "short size is", sizeof (short));
	msg ("%40s %3d byte", "int size is", sizeof (int));
	msg ("%40s %3d byte", "long size is", sizeof (long));
	msg ("%40s %3d byte", "long long size is", sizeof (long long));
	msg ("%40s %3d byte", "double size is", sizeof (double));
	msg ("%40s %3d byte", "word size is", sizeof (word));
}

/* Print version. */

static void
printversion (void)
{
	msg ("Library version is %d.%d", qmajor (), qminor ());
	if (debug > 0)
	{
		info ();
	}
}

/* Test. */
#ifdef TEST

static void
printhelp (void)
{
	fprintf (stdout, "\
This is a USB tester for Quantis RNG\n\
Usage:\n\
    qusb [-h][-v][-u][-d level][-b size][-l length][-n n] [cmd...]\n\
where\n\
    -h          prints this help.\n\
    -v          print version.\n\
    -u          unbuffered indirect.\n\
    -d level    debug level.\n\
    -b size     buffer size in bytes.\n\
    -l length   line length in characters.\n\
    -n n        repeat operation n times.\n\
    cmd         command to execute.\n\
Command is:\n\
    enumerate       Open and close\n\
    open            open the device.\n\
    readbulk        read one bulk packet.\n\
    read            read buffer as per -b.\n\
    byte            buffered read byte.\n\
    word            read word.\n\
    int             read int.\n\
    scaledint       read scaled int.\n\
    scaleddouble    read scaled double floating.\n\
    signal          test signal handling.\n\
    print           print buffer.\n\
    report          report device status.\n\
    probe           report if device connected.\n\
    close           close the device.\n\
");
	exit (FAILURE);
}

/* Main test program. */

int
main (int argc, char *argv[])
{
	char ch;
	char *options = "hvub:d:n:l:";
	char *op;
	int status = QUANTIS_ERROR_OTHER;
	qdev_t *q = NULL;
	int indirect = false;
	int buffersize = 0;
	char *buffer = NULL;
	int n = 0;
	int l = 80;
	int i = 0;
	int level = 0;
	byte b;
	word w;
	int rint;
	int min = 0;
	int max = 0;
	double rdouble;
	double fmin;
	double fmax;

	/* Deal with options. */
	opterr = 0;
	ch = getopt (argc, argv, options);
	while (ch != -1)
	{
		switch (ch)
		{
		case 'h':
			printhelp ();
			break;
		case 'v':
			printversion ();
			break;
		case 'u':
			indirect = true;
			break;
		case 'b':
			buffersize = atoi (optarg);
			if (buffersize == 0)
			{
				error ("Specify buffer size");
			}
			if (buffersize < 0)
			{
				error ("Wrong buffersize");
			}
			buffer = (char *) malloc (buffersize);
			if (buffer == NULL)
			{
				error ("No space for buffer");
			}
			dmsg ("Buffer size is %d", buffersize);
			break;
		case 'n':
			n = atoi (optarg);
			if (n == 0)
			{
				error ("Specify n");
			}
			if (n < 0)
			{
				error ("Wrong n");
			}
			dmsg ("Repeat number is %d", n);
			break;
		case 'l':
			l = atoi (optarg);
			if (l == 0)
			{
				error ("Specify length");
			}
			if (l < 0)
			{
				error ("Wrong length");
			}
			dmsg ("Line length is %d", l);
			break;
		case 'd':
			level = atoi (optarg);
			if (level == 0)
			{
				error ("Specify debug level (not 0)");
			}
			qdebug (level);
			dmsg ("Debug is %d", level);
			break;
		case '?':
			error ("Wrong option");
			break;
		default:
			error ("Wrong option - confused");
			break;
		}
		ch = getopt (argc, argv, options);
	}

	/* Create device descriptor. */
	q = qcreate ();
	if (q == NULL)
	{
		error ("Cannot create USB device descriptor");
	}

	/* Interpret arguments. */
	while (optind < argc)
	{
		op = argv[optind];
		if (seq (op, "openclose"))
		{

			/* Initialize, open and close. */
			status = qopen (q);
			if (status < 0)
			{
				error ("Error opening device");
			}
			qclose (q);
		}
		else if (seq (op, "open"))
		{
			status = qopen (q);
			if (status < 0)
			{
				error ("Error %d opening device", status);
			}
		}
		else if (seq (op, "readbulk"))
		{
			if (n == 0)
			{
				n = 1;
			}
			for (i=0; i<n; i++)
			{
				status = qreadbulk (q);
				if (status < 0)
				{
					error ("Error %d reading bulk", status);
				}
			}
		}
		else if (seq (op, "read"))
		{
			if (buffer == NULL)
			{
				error ("No buffer, specify -b");
			}
			if (n == 0)
			{
				n = 1;
			}
			for (i=0; i<n; i++)
			{
				status = qread (q, buffer, buffersize);
				if (status < 0)
				{
					error ("Error %d reading", status);
				}
			}
		}
		else if (seq (op, "byte"))
		{
			if (n == 0)
			{
				n = 1;
			}
			for (i=0; i<n; i++)
			{
				status = qbyte (q, &b);
				if (status < 0)
				{
					error ("Function qbyte returned error");
				}
				fprintf (stdout, "%02x", (unsigned char) b);
			}
			fprintf (stdout, "\n");
			qprintbuffer (q->packet, q->packetsize, "", 0, l);
		}
		else if (seq (op, "word"))
		{
			if (n == 0)
			{
				n = 1;
			}
			for (i=0; i<n; i++)
			{
				status = qword (q, &w);
				if (status < 0)
				{
					error ("Function qword returned error");
				}
				fprintf (stdout, "%016llx\n", (unsigned long long) w);
			}
		}
		else if (seq (op, "int"))
		{
			if (n == 0)
			{
				n = 1;
			}
			for (i=0; i<n; i++)
			{
				status = qint (q, &rint);
				if (status < 0)
				{
					error ("Function qword returned error");
				}
				fprintf (stdout, "%12d\n", rint);
			}
		}
		else if (seq (op, "scaledint"))
		{

			/* Get additional parameters. */
			if (argc - 2 < optind)
			{
				error ("Min and max should be specified");
			}
			optind++;
			dmsg ("Min is %s", argv[optind]);
			min = atoi (argv[optind]);
			optind++;
			dmsg ("Max is %s", argv[optind]);
			max = atoi (argv[optind]);
			if (max <= min)
			{
				error ("Max should be larger than min");
			}
			if (n == 0)
			{
				n = 1;
			}
			for (i=0; i<n; i++)
			{
				status = qscaledint (q, indirect, &rint, min, max);
				if (status < 0)
				{
					error ("Function qword returned error");
				}
				fprintf (stdout, "%12d\n", rint);
			}
		}
		else if (seq (op, "scaleddouble"))
		{

			/* Get additional parameters. */
			optind++;
			dmsg ("Min is %s", argv[optind]);
			fmin = atof (argv[optind]);
			optind++;
			dmsg ("Max is %s", argv[optind]);
			fmax = atof (argv[optind]);
			if (fmax <= fmin)
			{
				error ("Max should be larger than min");
			}
			if (n == 0)
			{
				n = 1;
			}
			for (i=0; i<n; i++)
			{
				status = qscaleddouble (q, indirect, &rdouble, fmin, fmax);
				if (status < 0)
				{
					error ("Function qscaleddouble returned error %d", status);
				}
				fprintf (stdout, "%32.12f\n", rdouble);
			}
		}
		else if (seq (op, "print"))
		{
			qprintbuffer (buffer, buffersize, " ", 0, l);
		}
		else if (seq (op, "report"))
		{
			qreport (q);
		}
		else if (seq (op, "info"))
		{
			info ();
		}
		else if (seq (op, "probe"))
		{
			if (qprobe ())
			{
				msg ("Device present");
			}
			else
			{
				msg ("Device not present");
			}
		}
		else if (seq (op, "signal"))
		{

			msg ("Establishing default signal handler");
			qsignal (q, NULL);
			msg ("Sleeping - waiting for signal");
			sleep (3);
			msg ("After sleep");
			qsignaldfl (q);
		}
		else if (seq (op, "close"))
		{
			qclose (q);
		}
		else
		{
			error ("Invalid command");
		}
		optind++;
	}

	/* Finish. Will close if it's open. */
	qdestroy (q);
	exit (SUCCESS);
}

#endif

/* End of file QUSB.C. */

