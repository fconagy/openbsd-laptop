#!/bin/sh

# Set up X devices for old-style Xorg run.
# This is done better with xenodm.

# Kill mouse daemon, not compatible with X.
PID="`ps -axu | grep wsmoused | grep -v grep | awk ' { print $2 } '`"
if [ "$PID" != "" ]
then
	kill $PID
fi

case $1 in
""|enable)

	# Enable access for xorg group.
	chgrp xorg /dev/ttyC*
	chmod g+rw /dev/ttyC*
	chgrp xorg /dev/drm*
	chmod g+rw /dev/drm*
	chgrp xorg /dev/dri/*
	chmod g+rw /dev/dri/*
	chgrp xorg /dev/wskbd*
	chmod g+rw /dev/wskbd*
	chgrp xorg /dev/wsmouse*
	chmod g+rw /dev/wsmouse*
	;;
reset)

	# Reset to the original ownership/permissions.
	chown root:wheel /dev/ttyC*
	chmod og-rw /dev/ttyC*
	chown root:wheel /dev/drm*
	chmod og-rw /dev/drm*
	chown root:wheel /dev/dri/*
	chmod og-rw /dev/dri/*
	chown root:wheel /dev/wskbd*
	chmod og-rw /dev/wskbd*
	chown root:wheel /dev/wsmouse*
	chmod g+rw /dev/wsmouse*
	;;
esac

