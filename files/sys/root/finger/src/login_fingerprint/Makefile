# Make file for login_fingerprint and enroll_fingerprint.
# Originally 2009 Robert Nagy, see copyright there.

# OpenBSD-like variables.
LOCALBASE=	/usr/local
X11BASE=	/usr/X11R6
PROG=		login_fingerprint
SRCS=		login_fingerprint.c
MAN=		login_fingerprint.8 enroll_fingerprint.1

DOCS=		README
DOCDIR=		$(LOCALBASE)/share/doc/login_fingerprint
MANDIR=		$(LOCALBASE)/man

CFLAGS+=	-Wall
CFLAGS+=	-I/usr/local/include/libusb-1.0
CFLAGS+=	-I/usr/local/include/libfprint
CFLAGS+=	-I${X11BASE}/include

LDADD+=		-L${X11BASE}/lib
LDADD+=		-L/usr/local/lib
LDADD+=		-lusb
LDADD+=		-lfprint
LDADD+=		-lMagickWand-6.Q16
LDADD+=		-lcrypto -lfreetype

BINDIR=		$(LOCALBASE)/bin
AUTHDIR=	/usr/libexec/auth

all: login_fingerprint enroll_fingerprint

# Objects.

login_fingerprint.o: login_fingerprint.c
	$(CC) $(CFLAGS) -c login_fingerprint.c

enroll_fingerprint.o: enroll_fingerprint.c
	$(CC) $(CFLAGS) -c enroll_fingerprint.c

# Executables.

enroll_fingerprint: enroll_fingerprint.o
	$(CC) -o enroll_fingerprint enroll_fingerprint.o $(LDADD)

login_fingerprint: login_fingerprint.o
	$(CC) -o login_fingerprint login_fingerprint.o $(LDADD)

# Clean.

clean:
	rm -f login_fingerprint login_fingerprint.o login_fingerprint.d
	rm -f enroll_fingerprint enroll_fingerprint.o enroll_fingerprint.d
	rm -f core *.core

distclean: clean

# Install and uninstall.

install: login_fingerprint enroll_fingerprint
	install -d -o root -g bin -m 755 $(AUTHDIR)
	install -d -o root -g bin -m 755 $(DOCDIR)
	install -c -o root -g bin -m 444 README $(DOCDIR)/
	install -c -o root -g bin -m 555 enroll_fingerprint $(BINDIR)/
	install -c -o root -g auth -m 555 login_fingerprint $(AUTHDIR)/
	install -c -o root -g bin -m 444 enroll_fingerprint.1 $(MANDIR)/man1/
	install -c -o root -g bin -m 444 login_fingerprint.8 $(MANDIR)/man8/

uninstall:
	rm $(DOCDIR)/README
	rm $(BINDIR)/enroll_fingerprint
	rm $(AUTHDIR)/login_fingerprint
	rm $(MANDIR)/man1/enroll_fingerprint.1
	rm $(MANDIR)/man8/login_fingerprint.8

# Save tarball.
save: distclean
	mkdir -p $(HOME)/save/login_fingerprint
	cd ..; \
		tar -z -cf $(HOME)/save/login_fingerprint/login_fingerprint.tar.gz ./login_fingerprint

