#!/bin/sh

# Check if all saved.

# Originals.
#FROM=/mnt/save00/save/tarsus/fconagy-home
#FROM=/mnt/save00/save/user61/users/fconagy
FROM=/mnt/save00/save/user71/users/fconagy

# Current home.
TO=~fconagy

# Archive on USB drive.
ARCH=/mnt/save00/save/naxos/user80/archive/fconagy

# All directories original.
cd $FROM
ls | while read F
do
	if [ -d $F ]
	then
		if [ -d $TO/$F ]
		then

			# Found at home.
			echo "$F home"
		elif [ -d $ARCH/$F ]
		then

			# Archived.
			echo "$F archive"
		else

			# Not found.
			echo "$F notfound"
		fi
	fi
done

# Finish.
exit 0

