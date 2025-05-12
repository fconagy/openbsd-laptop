#!/bin/sh

# Rebuild the kernel.

# To revert to an old kernel:
#mv /bsd /bsd.bad
#mv /bsd.orig /bsd
#sha256 -h /var/db/kernel.SHA256 /bsd

# Quit on error.
set -e

# Kernel name.
KNAME=CHIOS.MP

# Fast build when asked.
case $1 in
fast)

	# Fast build, after a full normal build.
	# No need to run config.
	cd /usr/src/sys/arch/amd64/compile/$KNAME
	make -DKERNFAST -j4 KERNCONF=$KNAME

	# Don't save just install.
	make install
	;;
"")

	# Normal build.
	cd /usr/src/sys/arch/amd64/conf
	config $KNAME
	cd ../compile/$KNAME
	make clean
	make depend
	make

	# Save the old kernel.
	cp -p /bsd /bsd.`date +%Y%m%dT%H%M%S`

	# Install new kernel.
	make install
	;;
*)
	echo "Wrong argument $1"
	exit 1
	;;
esac

# Finish.
echo "$KNAME finished"
exit 0

