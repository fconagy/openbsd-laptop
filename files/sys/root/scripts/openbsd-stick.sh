#!/bin/sh

# Create OpenBSD install USB stick on OpenBSD.

# Disk device to use.
DEV=sd2
DEVNAME=/dev/r${DEV}c
if [ ! -c $DEVNAME ]
then
	echo "Wrong device $DEVNAME"
	exit 1
fi

# Image file.
IMAGE=/h/users/fconagy/openbsd-76/install76-amd64.img
if [ ! -r $IMAGE ]
then
	echo "Cannot find $IMAGE"
	exit 1
fi

if [ "$1" = "" ]
then
	echo "Arguments: [stick][extra]"
	exit 1
fi

# Stop on error.
set -e

if [ "$1" = "stick" ]
then
	shift

	# Write.
	echo "`date` Writing $DEVNAME"
	sleep 3
	dd bs=1m if=/dev/zero of=$DEVNAME count=64
	dd bs=1m if=$IMAGE of=$DEVNAME
	echo "`date` Done writing $DEV"
fi

if [ "$1" = "extra" ]
then
	shift

	# Add an extra filesystem on the unused space.
	# Actually this was done manually and not tested to
	# run it within the script.

	# Set boundaries, leave start set size to the end of
	# the disk.
	echo "`date` Set boundaries"
	disklabel -E $DEV <<!EOF
b

*
w
q
!EOF

	# Add partition D.
	# Default for start.
	# Default for end.
	# Default for file system.
	echo "`date` Create disk slice d"
disklabel -E $DEV <<!EOF
a d



w
q
!EOF

	# Create file system.
	echo "`date` Create file system"
	newfs -O 2 -o space -m 1 /dev/r${DEV}d >openbsd-stick-extrafs-76.log 2>&1
	echo "`date` Finished with ${DEV}d"
fi

# Finish.
exit 0

