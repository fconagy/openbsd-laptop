#!/bin/sh

# Recompile after change in the sys tree.

# Quit on error.
set -e

# Kernel name.
NAME="`uname -v | sed 's/#.*$//'`"
if [ "$NAME" = "" ]
then
	echo "Cannot decide on kernel name"
	exit 1
fi

# Our kernel.
cd /usr/src/sys/arch/amd64/compile/$NAME

# Build.
make config
make
make install

# Device file.
if [ ! -c /dev/mixrng ]
then
	mknod /dev/mixrng c 101 0
fi

# Finish.
exit 0

