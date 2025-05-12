#!/bin/sh

# Prepare randomness.

# This will create new files.
# Then you have to boot from external media.
# Remove the old files, put the new files in place.

# Quit on error.
set -e

# File /var/db/host.random.
# Size 65536 bytes.
OLDRND=/var/db/host.random
RND=${OLDRND}.new

echo "Update the entropy pool"

# Don't overwrite.
if [ ! -w $RND ]
then

	# Create new.
	#echo "Create new $RND"
	#dd if=/dev/zero of=$RND bs=512 count=128 status=none

	# Use copy of the old one.
	cp -p $OLDRND $RND
fi

# Written in four parts.
echo "Generating $RND"

# From the Quantis device.
echo -n "1"
qrandom -b 512 -n 32 -o - | dd of=$RND seek=0 bs=512 count=32 status=none

# From the Quantis device.
echo -n "2"
qrandom -b 512 -n 32 -o - | dd of=$RND seek=32 bs=512 count=32 status=none

# Yubikey as a GPG smart card.
echo -n "3"
scdrnd -b 16384 2>/dev/null | dd of=$RND seek=64 bs=512 count=32 status=none

# Yubikey in PIV mode.
echo -n "4\n"
yrnd -b -n 16384 | dd of=$RND seek=96 bs=512 count=32 status=none

# The file /etc/random.seed will be used.
# Size is 512 bytes.
OLDRND=/etc/random.seed
RND=${OLDRND}.new

# Second file, same as above.
echo "Generating $RND"

# Don't overwrite.
if [ ! -w $RND ]
then
	echo "Create new $RND"

	# Create new.
	#dd if=/dev/zero of=$RND bs=512 count=1 status=none

	# Use copy of the old one.
	cp -p $OLDRND $RND
fi

# Written in four parts.

# From the Quantis device.
echo -n "1"
qrandom -b 128 -n 1 -o - | dd of=$RND seek=0 bs=128 count=1 status=none

# From the Quantis device.
echo -n "2"
qrandom -b 128 -n 1 -o - | dd of=$RND seek=1 bs=128 count=1 status=none

# Yubikey as a GPG smart card.
echo -n "3"
scdrnd -b 128 2>/dev/null | dd of=$RND seek=2 bs=128 count=1 status=none

# Yubikey in PIV mode.
echo -n "4\n"
yrnd -b -n 128 | dd of=$RND seek=3 bs=128 count=1 status=none

echo "Finished"
echo "New files for /var/db/host.random /etc/random.seed in place"
echo "Reboot from external media and move .new files in place"

# Done.
exit 0

