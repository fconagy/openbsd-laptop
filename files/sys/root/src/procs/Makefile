# Make file for the procs tools.

# Destination directories.
HOMEBIN=$(HOME)/bin
LOCALDIR=$(HOME)/local
BINDIR=$(LOCALDIR)/bin
MANDIR=$(LOCALDIR)/man

# Default target.
all: procs usleep

# Our program.
procs: procs.c
	cc -o procs procs.c -lkvm

# Utility.
usleep: usleep.c
	cc -o usleep usleep.c

instdirs:
	mkdir -p $(HOMEBIN) $(LOCALDIR) $(BINDIR) $(MANDIR) $(MANDIR)/man8

install: procs usleep instdirs
	cp -p procs $(BINDIR)/
	cp -p usleep $(BINDIR)/
	cp -p xps $(BINDIR)/
	cp -p procs.man $(MANDIR)/man8/procs.8
	cp -p usleep.man $(MANDIR)/man8/usleep.8

uninstall:
	rm $(BINDIR)/procs
	rm $(BINDIR)/usleep
	rm $(BINDIR)/xps
	rm $(MANDIR)/man8/procs.8
	rm $(MANDIR)/man8/usleep.8

# Test.
test: procs usleep
	echo ">>>> Tests"
	-./procs -h
	./procs
	./procs -k
	./procs -j
	./usleep -h
	-./usleep
	./usleep 2000000
	echo "<<<< Tests finished"

# Cleanup. Note that xps is a script, don't remove.
clean:
	rm -f procs usleep

distclean: clean

