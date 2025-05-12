#!/bin/sh

# Format 4TB Seagate USB disk.

# These are for ext2. Does not work with GPT label.
# Big disk so use 8k block size.
# Need large files so -O 1.
# -I since no disk label / partition table.
# -m 1 will force optimize for space.
# Use BSD label on the disk, GPT or MBR does not work.
# Create 'a' as the full disk. Set fstype to 'ext2fs'.
# This is the conservative one which is supplied with
# the system. Use this one.
# With block size 4K should be good to 16TB.
# Would not take 8k. Does not work.
#newfs_ext2fs -b 4096 -m 1 -O 1 -v save01 /dev/rsd3a
# This is the one which comes with the e2fsutils package.
# Does not work.
#mke2fs -t ext2 -b 4096 -m 1 -L save11 /dev/rsd3a
# This is standard UFS2. Works.
#newfs -O 2 -o space -m 1 /dev/rsd3a

# Disk 1.
echo Start `date`
fdisk sd3
disklabel sd3
newfs -O 2 -o space -m 1 /dev/rsd3a
echo Finished `date`

