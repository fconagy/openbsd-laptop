
/* QUSB.H Quantis USB commands header file. */

/* Original taken from the Quantis utilities source, see copyright there. */

#ifndef _QUSB_H
#define _QUSB_H

/* Error codes. */
#define SUCCESS 0
#define FAILURE 1

/* Boolean values. */
#define true ((int) 1)
#define false ((int) 0)

/* Print buffer format. */
#define UPPERCASE 1
#define SPACEDELIM 2

/* USB verndor id and device id. */
#define VENDOR_ID_ELLISYS 0x0ABA
#define DEVICE_ID_QUANTIS_USB 0x0102

/* Timeout for request in milliseconds. */
#define QUANTIS_USB_REQUEST_TIMEOUT 1000

/* Maximum size of a bulk packet. Should be
       64 bytes for USB 1
       512 bytes for USB 2
*/
#define USB_MAX_BULK_PACKETSIZE 512

/* Quantis USB Endpoint for bulk requests. */
#define QUANTIS_USB_ENDPOINT_BULK_IN 0x86

/* Quantis USB commands */
#define QUANTIS_USB_CMD_GET_BOARD_VERSION  0x10
#define QUANTIS_USB_CMD_MODULE_DISABLE     0x11
#define QUANTIS_USB_CMD_MODULE_ENABLE      0x12
#define QUANTIS_USB_CMD_GET_MODULES_STATUS 0x13
#define QUANTIS_USB_CMD_GET_MODULES_POWER  0x14
#define QUANTIS_USB_CMD_GET_MODULES_MASK   0x15
#define QUANTIS_USB_CMD_GET_MODULES_RATE   0x16

/* Error codes. */
#define QUANTIS_SUCCESS                         0
#define QUANTIS_ERROR_NO_DRIVER                -1
#define QUANTIS_ERROR_INVALID_DEVICE_NUMBER    -2
#define QUANTIS_ERROR_INVALID_READ_SIZE        -3
#define QUANTIS_ERROR_INVALID_PARAMETER        -4
#define QUANTIS_ERROR_NO_MEMORY                -5
#define QUANTIS_ERROR_NO_MODULE                -6
#define QUANTIS_ERROR_IO                       -7
#define QUANTIS_ERROR_NO_DEVICE                -8
#define QUANTIS_ERROR_OPERATION_NOT_SUPPORTED  -9
#define QUANTIS_ERROR_OTHER                   -99

/* Error code text. */
static char *qerrormessages[] = {

	/* From 0..-9 -> 0..9. */
	"Quantis success",
	"Quantis error no driver",
	"Quantis error invalid device number",
	"Quantis error invalid read size",
	"Quantis error invalid parameter",
	"Quantis error no memory",
	"Quantis error no module",
	"Quantis error io",
	"Quantis error no device",
	"Quantis error operation not supported",

	/* This is -99, anything larger or equal than 10 -> 10. */
	"Quantis error other"
};

/* Size for static strings. */
#define STRINGSIZE 255

/* No serial number. */
#define QUANTIS_NO_SERIAL "none"

/* No manufacturer. */
#define QUANTIS_NOT_AVAILABLE "n/a"

/* Version. */
#define MAJOR 0
#define MINOR 1

/* Word. */
typedef unsigned long word;

/* Byte. */
typedef unsigned char byte;

/* In OpenBSD the corresponding device names are
   like /dev/usbn, with the number being the bus number.
   The command 'usbdevs' lists the connected devices.
*/

/* Quantis device descriptor. */
typedef struct qdev
{
	int claimed;
	int opened;
	libusb_context *context;
	libusb_device **devices;
	libusb_device *dev;
	libusb_device_handle *handle;
	struct libusb_device_descriptor *desc;
	struct libusb_config_descriptor *usbconfig;
	char *serial;
	char *manufacturer;
	char *product;
	int version;
	int datarate;
	int busnumber;
	int deviceaddress;
	int pos;
	int packetsize;
	int endpoint;
	unsigned char *packet;
} qdev_t;

/* Prototype definitions. */

/* Report LIBUSB error. */
extern void qemsg (int code, const char *format, ...);

/* Set debug level. */
extern void qdebug (int level);

/* Print buffer. */
extern void qprintbuffer (char *buffer, int size,
	char *delim, int format, int linelength);

/* Create Quantis descriptor. */
extern qdev_t * qcreate (void);

/* Destroy Quantis descriptor. */
extern void qdestroy (qdev_t *q);

/* Declare signal handler. */
extern int qsignal (qdev_t *q, void (*handler)(int));

/* Put back default signal handler. */
extern int qsigdfl (qdev_t *q);

/* Set LIBUSB debug level. */
extern void qusbdebug (qdev_t *q, int level);

/* Enumerate Quantis device. */
extern int qenumerate (qdev_t *q);

/* Probe Quantis device. */
extern int qprobe (void);

/* Open device. */
extern int qopen (qdev_t *q);

/* Close. */
extern int qclose (qdev_t *q);

/* Read one packet with bulk transfer. */
extern int qreadbulk (qdev_t *q);

/* Read one byte buffered. */
extern int qbyte (qdev_t *q, byte *r);

/* Read one word, buffered. */
extern int qword (qdev_t *q, word *r);

/* Read integer buffered. */
extern int qint (qdev_t *q, int *r);

/* Read one word, unbuffered, will read one packet for it. */
extern int qwordui (qdev_t *q, word *r);

/* Read integer unbuffered indirect. */
extern int qintui (qdev_t *q, int *r);

/* Read scaled int. */
extern int qscaledint (qdev_t *q, int indirect, int *r,
	int min, int max);

/* Read scaled double. */
extern int qscaleddouble (qdev_t *q, int indirect, double *r,
	double min, double max);

/* Read. */
extern int qread (qdev_t *q, void *buffer, size_t size);

/* Situation report. */
extern void qreport (qdev_t *q);

/* Return version. */
extern int qmajor (void);
extern int qminor (void);

#endif

/* End of file QUSB.H. */

