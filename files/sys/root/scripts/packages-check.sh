#!/bin/sh

# Check packages.

# List of installed packages.
LIST=packages-check.list
ls /var/db/pkg | rev | cut -d '-' -f 2- | rev >$LIST

# Check if every package was installed as requested by the install script.
cat `dirname $0`/packages-install.sh | grep "^pkg_add [a-z]" | sed 's/pkg_add[ 	]*//' | \
	while read NAMES
do

	# Get the package names from the script.
	for N in $NAMES
	do
		case $N in
		*.tgz)

			# Ignore directly installed packages.
			;;
		*--*)

			# Special case for "--" names to select version.
			;;
		*)

			# Check if the package is in the local db.
			echo -n "$N"
			if cat $LIST | grep "^${N}\$" >/dev/null
			then
				echo " installed"
			else
				echo " notpresent"
			fi
			;;
		esac
	done
done
rm -f $LIST
exit 0

