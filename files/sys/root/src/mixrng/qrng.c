
/* QRNG.C is an OpenBSD device driver to mix randomness
   to the pool from a hopefully true random number generator
   device. */

/* System include files. */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/time.h>
#include <sys/timeout.h>

/* OpenBSD USB library includes. */
#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#include <dev/usb/usbdevs.h>

/* Device name macro. */
#define DEVNAME(_sc) ((_sc)->sc_dev.dv_xname)

/* Debug printout. */
#ifdef QRNG_DEBUG
#define DPRINTF(x)	printf x
#else
#define DPRINTF(x)
#endif

struct qrng_chip {
	int	bufsiz;
	int	endpoint;
	int	ctl_iface_idx;
	int	msecs;
	int	read_timeout;
};

/* Local variables. */
struct qrng_softc {
	struct  device		 sc_dev;
	struct  usbd_device	*sc_udev;
	struct  usbd_pipe	*sc_inpipe;
	struct  timeout 	 sc_timeout;
	struct  usb_task	 sc_task;
	struct  usbd_xfer	*sc_xfer;
	struct	qrng_chip	 sc_chip;
	int     		*sc_buf;
	int			 sc_product;
};

/* Device driver entry points. */
int qrng_match(struct device *, void *, void *);
void qrng_attach(struct device *, struct device *, void *);
int qrng_detach(struct device *, int);
void qrng_task(void *);
void qrng_timeout(void *);

/* Driver. */
struct cfdriver qrng_cd = {
	NULL, "qrng", DV_DULL
};

/* Match, attach, detach device. */
const struct cfattach qrng_ca =
{
	sizeof (struct qrng_softc),
	qrng_match,
	qrng_attach,
	qrng_detach
};

/* Device descriptor. */
struct qrng_type {
	struct usb_devno	qrng_dev;
	struct qrng_chip	qrng_chip;
};

/* Device descriptors. */
static const struct qrng_type qrng_devs[] = {

/* Quantis RNG. */
/* Should be endpoint 6. That is the bulk transfer endpoint. */
	{ { USB_VENDOR_IDQUANTIQUE, USB_PRODUCT_IDQUANTIQUE_QUANTISUSB },
	  {512, 6, 0, 100, 5000} },
};
#define qrng_lookup(v, p) ((struct qrng_type *)usb_lookup(qrng_devs, v, p))

int
qrng_match(struct device *parent, void *match, void *aux)
{
	struct usb_attach_arg *uaa = aux;

	if (uaa->iface == NULL)
		return (UMATCH_NONE);

	if (qrng_lookup(uaa->vendor, uaa->product) != NULL)
		return (UMATCH_VENDOR_PRODUCT);

	return (UMATCH_NONE);
}

/* Device connected, attach. */

void
qrng_attach (struct device *parent, struct device *self, void *aux)
{
	struct qrng_softc *sc = (struct qrng_softc *) self;
	struct usb_attach_arg *uaa = aux;
	usb_interface_descriptor_t *id;
	usb_endpoint_descriptor_t *ed;
	int ep_ibulk = -1;
	usbd_status error;
	int i, ep_addr;

	sc->sc_udev = uaa->device;
	sc->sc_chip = qrng_lookup(uaa->vendor, uaa->product)->qrng_chip;
	sc->sc_product = uaa->product;

	DPRINTF(("%s: bufsiz: %d, endpoint: %d ctl iface: %d, msecs: %d, read_timeout: %d\n",
		DEVNAME(sc),
		sc->sc_chip.bufsiz,
		sc->sc_chip.endpoint,
		sc->sc_chip.ctl_iface_idx,
		sc->sc_chip.msecs,
		sc->sc_chip.read_timeout));

	/* Find the bulk endpoints. */
	id = usbd_get_interface_descriptor(uaa->iface);
	for (i = 0; i < id->bNumEndpoints; i++) {
		ed = usbd_interface2endpoint_descriptor(uaa->iface, i);
		if (ed == NULL) {
			printf("%s: failed to get endpoint %d descriptor\n",
			    DEVNAME(sc), i);
			goto fail;
		}
		if (UE_GET_DIR(ed->bEndpointAddress) == UE_DIR_IN &&
		    UE_GET_XFERTYPE(ed->bmAttributes) == UE_BULK) {
		    	ep_addr = UE_GET_ADDR(ed->bEndpointAddress);

			DPRINTF(("%s: bulk endpoint %d\n",
			    DEVNAME(sc), ep_addr));

			if (ep_addr == sc->sc_chip.endpoint) {
				ep_ibulk = ed->bEndpointAddress;
				break;
			}
		}
	}

	if (ep_ibulk == -1) {
		printf("%s: missing bulk input endpoint\n", DEVNAME(sc));
		goto fail;
	}

	/* Open the pipes. */
	error = usbd_open_pipe(uaa->iface, ep_ibulk, USBD_EXCLUSIVE_USE,
		    &sc->sc_inpipe);
	if (error) {
		printf("%s: failed to open bulk-in pipe: %s\n",
				DEVNAME(sc), usbd_errstr(error));
		goto fail;
	}

	/* Allocate the transfer buffers. */
	sc->sc_xfer = usbd_alloc_xfer(sc->sc_udev);
	if (sc->sc_xfer == NULL) {
		printf("%s: could not alloc xfer\n", DEVNAME(sc));
		goto fail;
	}

	sc->sc_buf = usbd_alloc_buffer(sc->sc_xfer, sc->sc_chip.bufsiz);
	if (sc->sc_buf == NULL) {
		printf("%s: could not alloc %d-byte buffer\n", DEVNAME(sc),
				sc->sc_chip.bufsiz);
		goto fail;
	}

	/* And off we go! */
	usb_init_task(&sc->sc_task, qrng_task, sc, USB_TASK_TYPE_GENERIC);
	timeout_set(&sc->sc_timeout, qrng_timeout, sc);
	usb_add_task(sc->sc_udev, &sc->sc_task);

	return;

fail:
	usbd_deactivate(sc->sc_udev);
}

int
qrng_detach(struct device *self, int flags)
{
	struct qrng_softc *sc = (struct qrng_softc *)self;

	usb_rem_task(sc->sc_udev, &sc->sc_task);

	if (timeout_initialized(&sc->sc_timeout))
		timeout_del(&sc->sc_timeout);

	if (sc->sc_xfer != NULL) {
		usbd_free_xfer(sc->sc_xfer);
		sc->sc_xfer = NULL;
	}

	if (sc->sc_inpipe != NULL) {
		usbd_close_pipe(sc->sc_inpipe);
		sc->sc_inpipe = NULL;
	}

	return (0);
}


void
qrng_task(void *arg)
{
	struct qrng_softc *sc = (struct qrng_softc *)arg;
	usbd_status error;
	u_int32_t len, i;
#ifdef QRNG_MEASURE_RATE
	time_t elapsed;
	int rate;
#endif
	usbd_setup_xfer(sc->sc_xfer, sc->sc_inpipe, NULL, sc->sc_buf,
	    sc->sc_chip.bufsiz, USBD_SHORT_XFER_OK | USBD_SYNCHRONOUS,
	    sc->sc_chip.read_timeout, NULL);

	error = usbd_transfer(sc->sc_xfer);
	if (error) {
		printf("%s: xfer failed: %s\n", DEVNAME(sc),
		    usbd_errstr(error));
		goto bail;
	}

	usbd_get_xfer_status(sc->sc_xfer, NULL, NULL, &len, NULL);
	if (len < sizeof(int)) {
		printf("%s: xfer too short (%u bytes) - dropping\n",
		    DEVNAME(sc), len);
		goto bail;
	}

#ifdef QRNG_MEASURE_RATE
	if (sc->sc_first_run) {
		sc->sc_counted_bytes = 0;
		getmicrotime(&(sc->sc_start));
	}
	sc->sc_counted_bytes += len;
	getmicrotime(&(sc->sc_cur));
	elapsed = sc->sc_cur.tv_sec - sc->sc_start.tv_sec;
	if (elapsed >= QRNG_RATE_SECONDS) {
		rate = (8 * sc->sc_counted_bytes) / (elapsed * 1024);
		printf("%s: transfer rate = %d kb/s\n", DEVNAME(sc), rate);

		/* set up for next measurement */
		sc->sc_counted_bytes = 0;
		getmicrotime(&(sc->sc_start));
	}
#endif

	len /= sizeof(int);
	for (i = 0; i < len; i++) {
		enqueue_randomness(sc->sc_buf[i]);
	}
bail:
#ifdef QRNG_MEASURE_RATE
	if (sc->sc_first_run) {
		sc->sc_first_run = 0;
	}
#endif

	timeout_add_msec(&sc->sc_timeout, sc->sc_chip.msecs);
}

void
qrng_timeout(void *arg)
{
	struct qrng_softc *sc = arg;

	usb_add_task(sc->sc_udev, &sc->sc_task);
}
