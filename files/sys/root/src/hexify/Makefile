# Make file for hexify. Small utility to convert binary
# stdin to printout in hex.

all: hexify

hexify: hexify.c
	cc -o hexify hexify.c

test: hexify
	dd if=/dev/urandom bs=512 count=1 2>/dev/null | ./hexify
	echo "" | ./hexify
	echo -n "" | ./hexify
	-./hexify -n
	-./hexify -x
	dd if=/dev/urandom bs=512 count=1 2>/dev/null | ./hexify -u
	dd if=/dev/urandom bs=512 count=1 2>/dev/null | ./hexify -l
	dd if=/dev/urandom bs=512 count=1 2>/dev/null | ./hexify -n 8 -l
	echo "Tests finished"

clean:
	rm -f hexify hexify.core

distclean: clean

install: hexify
	cp hexify $(HOME)/local/bin/

uninstall:
	rm -f $(HOME)/local/bin/hexify

