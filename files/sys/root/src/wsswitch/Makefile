# Make file for wsswitch.

# Default.
all: wsswitch

# Our only program.
wsswitch: wsswitch.c
	cc -Wall -o wsswitch wsswitch.c

clean:
	rm -f wsswitch

distclean: clean

install: wsswitch
	cp wsswitch $(HOME)/bin/

uninstall:
	rm $(HOME)/bin/wsswitch

