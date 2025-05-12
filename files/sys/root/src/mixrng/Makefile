# Make file for MIXRNG test program.

# Default target.
all: mixrng-test

# Test program.
mixrng-test: mixrng-test.c
	cc -o mixrng-test mixrng-test.c

# Cleanup.
clean:
	rm -f mixrng-test
	rm -f driver-compile.log

distclean: clean

clobber: clean

# Create device file.
makedev:
	./mixrng-makedev.sh

# Run tests.
test: makedev mixrng-test
	echo ">>>> Running tests"
	./mixrng-test open close
	./mixrng-test open debug close
	-./mixrng-test read
	-./mixrng-test open read close
	./mixrng-test open write close
	-./mixrng-test -n 8 open read close
	./mixrng-test -n 8 open write close
	./mixrng-test -b 13 open write close
	./mixrng-test -b 1030 open write close
	-./mixrng-test -b 1030 open read close
	./mixrng-test open ioctl close
	./mixrng-test open nodebug close
	./mixrng-test -n 10 open nodebug write close
	./mixrng-test open debug close
	./mixrng-test -n 100 open nodebug write stat close
	./mixrng-test open debug stat close
	date
	tail /var/log/messages
	echo "<<<< Tests finished"

# Update copies here with changes in /usr/src.
update:
	./update-from.sh

# Build kernel with driver changed.
driver:
	./driver-compile.sh 2>&1 | tee driver-compile.log

# Test.
tm:
	make test 2>&1 | more

# Create save tarball.
save: distclean
	mkdir -p $(HOME)/save/mixrng; \
	cd ..; \
	tar -z -cf $(HOME)/save/mixrng/mixrng.tar.gz ./mixrng

