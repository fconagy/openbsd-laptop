#!/bin/sh

# Delete packages accordingly to /var/log/messages excerpts.

# Part of /var/log/messages with pkg_add messages about
# installing dependencies and package.
LIST=git-cola-dependencies.list

# Get the lines from pkg_add and reverse the order of lines.
cat $LIST | grep 'pkg_add:' | tail -r | awk ' { print $7 } ' | \
	while read PKG
do

	# Package name to deinstall.
	pkg_delete $PKG
	if [ -d /var/db/pkg/$PKG ]
	then

		# Installed.
		pkg_delete $PKG
		if [ $? -eq 0 ]
		then
			echo "$PKG deleted"
		else
			echo "$PKG error-deleting"
		fi
	fi
done

exit 0

