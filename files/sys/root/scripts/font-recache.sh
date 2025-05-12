#!/bin/sh

# Re-cache fonts after added a new font.

# List of font directories.
FONTDIRS="/usr/local/share/fonts"

# For all directories.
for D in $FONTDIRS
do

	# Get subdirectories at level 1.
	for DD in $D/*
	do

		# Only directories.
		if [ -d $DD ]
		then
			echo $DD
			cd $DD
			mkfontdir -b
			mkfontscale -b
		fi
	done
done
fc-cache -r -f -v -s

# Finish.
exit 0

