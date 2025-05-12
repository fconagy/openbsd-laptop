# MAKEFILE is for the Quantis hardware random number generator.

# Destination directories.
HOMEBIN=$(HOME)/bin
LOCALDIR=$(HOME)/local
BINDIR=$(LOCALDIR)/bin
MANDIR=$(LOCALDIR)/man

# Include directory paths.
# LIBUSB used.
USBINCDIR=/usr/local/include
QUSBINCDIR=./include
INC=-I$(QUSBINCDIR) -I$(USBINCDIR)

# Library directory and my Quantis libraries.
QUSBLIBDIR=./lib
QUSBLIB=$(QUSBLIBDIR)/libqusb.a
LIB=-L$(QUSBLIBDIR) -lqusb

# USB library. Note that it is not -lusb.
# This is LIBUSB.
USBLIBDIR=/usr/local/lib
USB=-L $(USBLIBDIR) -lusb-1.0

# C compiler.
# This was under FreeBSD.
#CC=gcc -Wl,-rpath=/usr/local/lib/gcc9
# Current on OpenBSD.
CC=cc -Wall -Wno-unused-function

# C compiler flags.
# For debug.
#CFLAGS=-g $(INC)
# Under OpenBSD.
CFLAGS=$(INC)

# Loader flags.
LDFLAGS=$(USB) $(LIB) -pthread -static

# Default pseudo target.
all: qrandom qrngd

# My own Quantis USB library.
# Build when it's not there.
$(QUSBLIB): $(QUSBINCDIR)/qusb.h $(QUSBLIBDIR)/qusb.c
	cd $(QUSBLIBDIR); \
	make

# Create library when needed.
lib: $(QUSBLIB)

# Random number generator utility.
qrandom: $(QUSBLIB) qrandom.c
	$(CC) -o $@ $(CFLAGS) qrandom.c $(LDFLAGS)

# Daemon.
qrngd: $(QUSBLIB) qrngd.c
	$(CC) -o $@ $(CFLAGS) qrngd.c $(LDFLAGS)

# Library cleanup.
libclean:
	cd $(QUSBLIBDIR); \
	make clean

# Cleanup.
clean:
	rm -f qrandom qrngd
	rm -f qrandom.core qrngd.core
	rm -f -P x.x

# Other usual targets.

distclean: clean libclean

clobber: clean

# Install and uninstall.

installdirs:
	mkdir -p $(HOMEBIN)
	mkdir -p $(LOCALDIR)
	mkdir -p $(BINDIR)
	mkdir -p $(MANDIR)
	mkdir -p \
$(MANDIR)/man8

install: installdirs qrandom qrngd
	cp -p qrandom $(BINDIR)/
	cp -p qrandom.man $(MANDIR)/man8/qrandom.8
	cp -p qrngd $(BINDIR)/
	cp -p qrngd.man $(MANDIR)/man8/qrngd.8

uninstall:
	rm $(BINDIR)/qrandom
	rm $(MANDIR)/man8/qrandom.8
	rm $(BINDIR)/qrngd
	rm $(MANDIR)/man8/qrngd.8

libinstall:
	cd $(QUSBLIBDIR); \
	make; \
	make install

libuninstall:
	cd $(QUSBLIBDIR); \
	make uninstall

# Tests.

# Library test.
libtest:
	cd $(QUSBLIBDIR); \
	make; \
	make test

# Test.
test: qrandom qrngd
	echo ">>> Tests"
	rm -f -P x.x
	./qrandom -l
	-./qrandom -h
	-./qrandom -o x.x
	-./qrandom -b 1000000 -o x.x
	-./qrandom -b asdf -o x.x
	-./qrandom -b -3 -o x.x
	./qrandom -b 100 -o x.x
	-./qrandom -b 100 -o x.x
	./qrandom -b 100 -o - >x.x
	rm -P x.x
	./qrandom -b 100000 -o x.x
	ls -l x.x
	rm -P x.x
	./qrandom -n 16 -b 1024 -o x.x
	rm -P x.x
	./qrandom -b 72 -p
	./qrandom -n 9 -b 72 -p
	./qrandom -b 80 -p | wc -c
	./qrandom -b 891 -p
	./qrandom -b 1024 -m
	-./qrandom -b 1024 -x
	-./qrandom -i x,3
	-./qrandom -i 1,x
	-./qrandom -i
	./qrandom -n 32 -i 1,6
	./qrandom -n 32 -d
	./qrandom -n 2 -b 32 -c 1
	./qrandom -n 10 -b 64 -c 2
	./qrandom -n 10 -b 64 -c 3
	./qrandom -n 10 -b 32 -c 4
	./qrandom -n 10 -b 32 -c 4
	./qrandom -n 10 -b 32 -c 4
	./qrandom -n 10 -b 32 -c 4
	./qrandom -n 10 -b 32 -c 5
	./qrandom -n 10 -b 32 -c 6
	./qrandom -n 20 -b 50 -u -c 5
	./qrandom -b 128 -n 1 -p
	./qrandom -b 128 -n 1 -s -p
	./qrandom -b 128 -n 1 -U -p
	./qrandom -b 128 -n 1 -s -U -p
	-./qrngd -h
	-./qrngd -v -d 99 -s 0.3 &
	sleep 2; \
	kill -HUP $$(ps | grep ./qrngd | grep -v grep | awk ' { print $$1 } ')
	# Waiting to finish.
	sleep 5
	rm -f /var/run/qrngd.pid
	-ps -axu | grep ./qrngd | grep -v grep
	-./qrngd -v -d 1 -s 0.3 &
	sleep 2; \
	./qrngd -k ; sleep 3
	tail -48 /var/log/qrngd.log
	echo ">>> Tests finished"

tm:
	make test 2>&1 | more

# Create save tarball.
save: distclean
	mkdir -p $(HOME)/save/qrandom; \
	cd ..; \
	tar -z -cf $(HOME)/save/qrandom/qrandom.tar.gz ./qrandom

# End of make file.

