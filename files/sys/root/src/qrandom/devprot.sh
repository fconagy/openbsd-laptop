#!/bin/sh

# Manipulates device access to Enable users to use usb devices via libusb.
# Note that this gives access to ALL usb devices so it is to be used with
# caution, only while debugging.

# Bail out on error.
set -e

OP="$1"
case $OP in
"")
	echo "Specify enable, disable or list"
	exit 1
	;;
enable)
	GROUP=usb
	ls /dev/ugen* | while read DEV REST
	do
		chgrp $GROUP $DEV
		chmod g+rw $DEV
	done
	ls /dev/usb* | while read DEV REST
	do
		chgrp $GROUP $DEV
		chmod g+rw $DEV
	done
	;;
disable)
	ls /dev/ugen* | while read DEV REST
	do
		chgrp wheel $DEV
		chmod g-rw $DEV
	done
	ls /dev/usb* | while read DEV REST
	do
		chgrp wheel $DEV
		chmod g-w $DEV
	done
	;;
list)
	ls /dev/ugen* | while read DEV REST
	do
		ls -l $DEV
	done
	ls /dev/usb* | while read DEV REST
	do
		ls -l $DEV
	done
	;;
*)
	echo "$OP unknown"
	exit 1
	;;
esac
exit 0

