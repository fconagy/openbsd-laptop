# MAKEFILE is the make file for yrnd.

LOCALDIR=$(HOME)/local
BINDIR=$(LOCALDIR)/bin
MANDIR=$(LOCALDIR)/man

all: yrnd

yrnd: yrnd.c
	cc -I/usr/local/include \
		-I/usr/local/include/PCSC \
		-L /usr/local/lib -o yrnd yrnd.c -lpcsclite

# Cleanup. Note that scdrnd is a script, do not remove.
clean:
	rm -f yrnd yrnd.core

distclean: clean

installdirs:
	mkdir -p $(LOCALDIR) $(BINDIR) $(MANDIR) $(MANDIR)/man8

install: installdirs yrnd
	cp -p yrnd $(BINDIR)/
	cp -p yrnd.man $(MANDIR)/man8/yrnd.8
	cp -p scdrnd $(BINDIR)/

uninstall:
	rm $(BINDIR)/yrnd
	rm $(MANDIR)/man8/yrnd.8
	rm $(BINDIR)/scdrnd

test: yrnd
	echo ">>>> Tests"
	-./yrnd -h
	./yrnd
	./yrnd -n 8
	./yrnd -n 10
	./yrnd -n 100
	./yrnd -b -n 10 >x.x
	hexdump -C x.x
	ls -l x.x
	rm x.x
	./yrnd -v
	echo "<<<< Tests finished"

# Create save tarball.
save: distclean
	mkdir -p $(HOME)/save/yrnd; \
	cd ..; \
	tar -z -cf $(HOME)/save/yrnd/yrnd.tar.gz ./yrnd

