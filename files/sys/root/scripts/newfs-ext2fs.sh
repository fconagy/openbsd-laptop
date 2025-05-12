#!/bin/sh

# Format 2TB USB disk.

# The maximum size for MBR partition is 2TiB.
# These are for ext2. Does not work with GPT label.
# Big disk so use 8k block size.
# Need large files so -O 1.
# -m 1 will force optimize for space.
# Use fdisk in interactive mode.
#   fdisk -e sd5
#   reinit mbr
#   edit 3
#     Disable it setting type to 0 as suggested.
#   edit 4
#     Use partition type 85 as Linux ext.
#     Start at 2048 and use * to select the whole disk.
# This will create 'a' as the full disk. Will set fstype to 'ext2fs'.

# To create the file system.
# This is the conservative one which is supplied with
# the system. Use this one.
# With block size 4K should be good to 16TB.
# Specify -I for new label.
#   newfs_ext2fs -I -b 4096 -m 1 -O 1 -v pic01 /dev/rsd5i

# Disk for pictures saved from phones.
# Note that -b 4096 nad -V 4 are defaults.
# The maximum size of a file system is 2GB x 512.
# That is 2147483648 512 byte blocks.
# Use fdisk -y -g sd5 to create the partition table.
# Then add the partitions with the size above and type
# ext2fs.
echo Start `date`
fdisk sd5
disklabel sd5
#newfs_ext2fs -I -b 4096 -V 4 -m 1 -O 1 -v pic00 /dev/rsd5i
newfs_ext2fs -I -b 4096 -V 4 -m 1 -O 1 -v pic01 /dev/rsd5j
echo Finished `date`

