#!/bin/sh

# Save the top of the disk.

DISK=/dev/r${1}c
if [ ! -c $DISK ]
then
	echo "Cannot find $DISK"
	exit 1
fi
dd if=$DISK of=disk-64-blocks.dd bs=512 count=64
dd if=$DISK of=disk-2048-blocks.dd bs=512 count=2048

