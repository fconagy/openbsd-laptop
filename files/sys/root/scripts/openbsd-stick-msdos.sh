#!/bin/sh

# Format MS-DOS disk.

# Disk.
DISK=sd5
if [ ! -c /dev/r${DISK}c ]
then
	echo "No device file for $DISK"
	exit 1
fi

# MS-DOS disk label. 8 characters.
#LABEL=CAMERA1
LABEL=ASLSONGS

# Log.
LOG=openbsd-stick-msdos-`echo ${LABEL} | awk ' { print tolower($1) } '`.log

# Quit on error.
set -e

# Wipe disk partition table.
dd if=/dev/zero of=/dev/r${DISK}c bs=1m count=16

# Create empty partition table. No boot code when it's just zeroes.
# Will create partition three as OpenBSD.
fdisk -y -i ${DISK}

# Edit partition table. Should have one OpenBSD partition.
# Partition type is '0C'. That is Win95 FAT32L. Change it from OpenBSD.
# Leave CHS mode as no, the default.
# Leave offset, should be 64, that should leave some space on top.
# Partition size had been defined as all disk already when it was initialized
#   to OpenBSD partition, so leave that.
# Write partition table.
# Quit.
# Prompts below:
# Partition type: 0C
# Edit in CHS mode: [n]:
# Partition offset [64]:
# Partition size [nnnnnnn]:
# w
# q
# Input is from here document, as above as below.
fdisk -e ${DISK} <<!EOF
edit 3
0C



w
q
!EOF

# Partitioning done, now format.
# MS-DOS partition seen as OpenBSD partition 'i'.
newfs_msdos -F 32 -L "${LABEL}" ${DISK}i 2>&1 | tee $LOG

# Show what we got.
fdisk $DISK | tee -a $LOG
disklabel $DISK | tee -a $LOG

# Finish.
exit 0

