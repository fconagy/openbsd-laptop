#!/bin/sh

# Update local copies from the kernel source files.
# Called from the make file.

# Copy when newer.

cpwn() {
	F="$1"
	T="`basename $F`"
	if [ ! -r $F ]
	then
		echo "Cannot access $F"
		exit 1
	fi
	if [ $F -nt $T ]
	then
		cp -p $F $T
		if [ $? -ne 0 ]
		then
			echo "Error copying $F"
			exit 1
		else
			echo "$T updated"
		fi
	else
			echo -n "$T up to date"
			if diff $F $T
			then
				echo -n " same"
			else
				echo -n " differs"
			fi
			printf "\n"
	fi
}

cpwn /usr/src/sys/dev/mixrng.c
cpwn /usr/src/sys/dev/mixrngio.h
cpwn /usr/src/sys/arch/amd64/conf/`uname -v | sed 's/#.*$//'`
cpwn /usr/src/sys/dev/usb/qrng.c
cpwn /usr/src/sys/conf/files
cpwn /usr/src/etc/MAKEDEV.common
cpwn /usr/src/etc/etc.amd64/MAKEDEV.md
cpwn /usr/src/sys/sys/conf.h
cpwn /usr/src/sys/arch/amd64/amd64/conf.c

exit 0

