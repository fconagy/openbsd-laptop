#!/bin/sh

# Format 1TB Toshiba internal disk.

# Internal spinning disk.
# The disk has only BSD label.
# No partition table.
echo Start `date`
fdisk sd0
disklabel sd0
newfs -O 2 -o space -m 1 /dev/rsd0a
echo Finished `date`

