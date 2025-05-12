#!/bin/sh

# Make a list of files which are not in the set.

cd /h/pkg-cache
for SET in *.tgz
do
	LIST=$SET.list
	if [ ! -r $LIST ]
	then
		tar -z -tf $SET >$LIST
	fi
	cat $LIST | sed 's/^\.//' | while read F
	do
		if [ ! -r $F ]
		then
			echo $F
		fi
	done
	rm $LIST
done

