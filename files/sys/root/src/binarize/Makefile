# Make file for binarize. Small utility to convert hex
# printout to binary.

all: binarize

binarize: binarize.c
	cc -o binarize binarize.c

test: binarize
	echo "aa bb cc dd" | ./binarize | hexdump -C
	echo "" | ./binarize
	echo "01 02 03 4455" | ./binarize | hexdump -C
	-echo "ll" | ./binarize
	echo "Tests finished"

clean:
	rm -f binarize binarize.core

distclean: clean

install: binarize
	cp binarize $(HOME)/local/bin/

uninstall:
	rm $(HOME)/local/bin/binarize

