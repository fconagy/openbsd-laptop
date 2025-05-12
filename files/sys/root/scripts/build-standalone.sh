#!/bin/sh

# Build the standalone stuff.

# EFI boot loader partition.
EFIDEV=/dev/sd1i

# Boot device.
BOOTDEV=sd1

# Quit on error.
set -e

# Build.
cd /usr/src/sys/arch/amd64/stand
make clean
make
make install
sleep 2

# Update the EFI loader.
# Actually this is done by installboot when there is an EFI boot
# setup as well.
#echo "Update EFI loader on $EFIDEV"
#mount $EFIDEV /mnt
#cp /usr/mdec/BOOTX64.EFI /mnt/efi/BOOT/
#cp /usr/mdec/BOOTIA32.EFI /mnt/efi/BOOT/
#umount /mnt

# Update BIOS boot loader.
echo "Update BIOS boot loader on $BOOTDEV"
installboot -v $BOOTDEV /usr/mdec/biosboot /usr/mdec/boot

