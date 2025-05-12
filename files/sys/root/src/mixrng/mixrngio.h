
/* MIXRNGIO.H is the header file for the mixrng device. */

/* System include files. */
#include <sys/param.h>
#include <sys/device.h>

/* Do not include twice. */
#ifndef MIXRNGIO_H
#define MIXRNGIO_H 1

/* String length in test param block. */
#define MIXRNGIO_DATA_LENGTH 512

/* Argument block for ioctl. */
struct mixrng_params
{
	int code;
	u_char data[MIXRNGIO_DATA_LENGTH];
};

/* IOCTL command codes. */

/* Test. */
#define MIXRNGIO_TEST _IOW('S', 0x1, struct mixrng_params)

/* Set debug level. Greater than zero means debug on. */
#define MIXRNGIO_DEBUG _IOW('S', 0x2, struct mixrng_params)

/* Print statistics in syslog. */
#define MIXRNGIO_STAT _IOW('S', 0x3, struct mixrng_params)

#ifdef _KERNEL

/* Empty. */

#endif
#endif

/* End of file MIXRNGIO.H. */

