#!/bin/sh

# Create Debian install USB stick on OpenBSD.

# Disk device to use.
DEV=sd5
DEVNAME=/dev/r${DEV}c
if [ ! -c $DEVNAME ]
then
	echo "Wrong device $DEVNAME"
	exit 1
fi

# Write.
echo "`date` Writing $DEVNAME"
sleep 3
dd bs=1m if=/h/users/fconagy/debian-12.2.0/debian-12.2.0-amd64-DVD-1.iso of=$DEVNAME
echo "`date` Done writing $DEV"

