#!/bin/sh

# GVFS disable / enable back.

SAVE=/usr/local/libexec/save
if [ ! -d $SAVE ]
then
	mkdir $SAVE
	if [ $? -ne 0 ]
	then
		echo "Cannot create save directory"
		exit 1
	fi
fi
case $1 in
disable)
	mv /usr/local/libexec/gvfs* ${SAVE}/
	;;
enable)
	mv $SAVE/gvfs* /usr/local/libexec/
	;;
*)
	echo "Unknown function $1"
	exit 1
	;;
esac

