# MAKEFILE is the make file for the qusb library and utility.
# This is under OpenBSD.

# Destination directories.
LOCALDIR=$(HOME)/local
LOCALLIB=$(LOCALDIR)/lib
LOCALINC=$(LOCALDIR)/include
LOCALBIN=$(LOCALDIR)/bin
LOCALMAN=$(LOCALDIR)/man

# C compiler.
#CC=egcc
CC=cc -Wall -Wno-unused-function -g

# Include root for the LIBUSB library.
INCUSB=/usr/local/include

# Include directory for our include file.
INCDIR=../include

# Library directory for LIBUSB.
LIBUSB=/usr/local/lib

# Libraries. Note 1.0.
LIBS=-lusb-1.0

# Defines.
# TEST to create a test executable.
# DEBUG to set the debug level compiled in.
# DEBUG should be defined as a number.
DEFTEST=-DTEST -DDEBUG=0
DEFLIB=-DDEBUG=0

# Compiler switches for test executable.
CSWITCHTEST=-g -I$(INCUSB) -I$(INCDIR) $(DEFTEST) -L$(LIBUSB)

# Compiler switches to create libraries.
CSWITCHLIB=-I$(INCUSB) -I$(INCDIR) $(DEFLIB)

# First pseudo target.
all: libqusb.a libqusb.so.1.0 qusb

# Create library.
libqusb.a: $(INCDIR)/qusb.h qusb.c
	$(CC) $(CSWITCHLIB) -c qusb.c
	rm -f libqusb.a
	ar rc libqusb.a qusb.o

# Create shared library.
libqusb.so.1.0: $(INCDIR)/qusb.h qusb.c
	$(CC) $(CSWITCHLIB) -fPIC -c qusb.c
	cc -shared -fPIC -o libqusb.so.1.0 qusb.o

# Create test executable.
qusb: $(INCDIR)/qusb.h qusb.c
	$(CC) $(CSWITCHTEST) -o qusb qusb.c $(LIBS)

# Cleanup.
clean:
	rm -f qusb qusb.o libqusb.a libqusb.so.1.0
	rm -f core core.* qusb.core
	rm -f x.x

distclean: clean

clobber: clean

# Install and uninstall.

installdirs:
	mkdir -p $(LOCALLIB)
	mkdir -p $(LOCALBIN)
	mkdir -p $(LOCALINC)
	mkdir -p $(LOCALMAN)
	mkdir -p $(LOCALMAN)/man8

install: installdirs
	cp -p libqusb.a $(LOCALLIB)/
	cp -p libqusb.so.1.0 $(LOCALLIB)/
	cp -p $(INCDIR)/qusb.h $(LOCALINC)/
	cp -p qusb $(LOCALBIN)/
	cp -p qusb.man $(LOCALMAN)/man8/qusb.8

uninstall:
	rm $(LOCALLIB)/libqusb.a
	rm $(LOCALLIB)/libqusb.so.1.0
	rm $(LOCALINC)/qusb.h
	rm $(LOCALBIN)/qusb
	rm $(LOCALMAN)/man8/qusb.8

# Run tests.
test: qusb
	@echo ">>>> Running tests"
	-./qusb -h
	-./qusb -d 0
	-./qusb -d xxx
	-./qusb xxx
	./qusb -d 3 openclose
	./qusb probe
	./qusb -d 3 open readbulk readbulk close
	./qusb -d 3 -b 512 open read close
	./qusb -d 3 -b 512 open read read read close
	./qusb -d 3 -b 17 open read read read close
	./qusb -d 3 -b 1024 open read read close
	./qusb -d 3 -b 1029 open read read read close
	-./qusb -d 3 -b 1024 read
	./qusb -d 3 -l 120 -b 1024 open read print close
	./qusb open report close
	./qusb -n 1024 open byte close
	./qusb -n 64 open word close
	./qusb -n 64 open scaledint 1 6 close
	./qusb -n 64 open scaledint 0 5 close
	./qusb -n 64 open scaleddouble 0.0 1.0 close
	./qusb -b 1024 -l 20 open read print close
	./qusb -b 1000 -l 40 open read print close
	./qusb -b 1000 -l 80 open read print close
	./qusb -b 500 -l 120 open read print close
	# This one is a bit tricky.
	# We send a signal to the background process.
	# Note the strange substitution.
	# Will not work with all makes.
	-./qusb -d 99 open signal close &
	sleep 2; \
	kill -HUP $$(ps | grep ./qusb | grep signal | grep -v grep | awk ' { print $$1 } ')
	# Wait until finish.
	sleep 5
	@echo "<<<< Tests finished"

tm:
	make test 2>&1 | more

