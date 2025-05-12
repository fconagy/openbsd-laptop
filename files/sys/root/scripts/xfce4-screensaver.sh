#!/bin/sh

# Disable or enable back xfce4-screensaver/screenlock.
# Kicks in even when there is an another lock configured.
# Also seems to lock up the whole system.
# Can't be removed since it wants to remove the whole xfce.

# Enable/disable.
case $1 in
disable)
	CMD="chmod -x"
	MSG="disabling"
	;;
enable)
	CMD="chmod +x"
	MSG="enabling"
	;;
*)
	echo "Unknown function $1"
	exit 1
	;;
esac

# Get a list of all executables and chmod them.
pkg_info -L xfce4-screensaver | grep '^/' | egrep -e '/bin/|/libexec/' | \
	while read F REST
do
	echo "$F $MSG"
	$CMD $F
	if [ $? -ne 0 ]
	then
		echo "Error running $CMD $F"
		exit 1
	fi
done

