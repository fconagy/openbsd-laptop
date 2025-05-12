#!/bin/sh

# Recompile after change in the sys tree.

# Quit on error.
set -e

# Device file.
if [ ! -c /dev/mixrng ]
then
	mknod /dev/mixrng c 101 0
fi

# Finish.
exit 0

