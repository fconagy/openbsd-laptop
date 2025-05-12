#!/bin/sh

# Retrieve installed sets.
# Use a list of recently installed software.
# Names obtained like
# grep 'pkg_add: Added' /var/log/messages | awk ' { print $7 } '

# Package cache directory.
CACHE=$HOME/pkg-cache
if [ ! -d $CACHE ]
then
	echo "Cannot find $CACHE"
	exit 1
fi
cd $CACHE

# Recently installed packages.
LIST=$CACHE/packages-recent-`date +%Y%m%dT%H%M%S`.list

# Remember the list.
grep 'pkg_add: Added' /var/log/messages | awk ' { print $1, $2, $3, $7 } ' >>$LIST

# The list is from the current syslog messages.
grep 'pkg_add: Added' /var/log/messages | awk ' { print $7 } ' | \
while read PKG
do
	case $PKG in
	*\-\>*)

		# Package was upgraded. Name and version.
		NAME="`echo $PKG | sed 's/->.*$//'`"
		NAME="`echo $NAME | sed 's/[^-]*$//; s/\-$//'`"
		VERS="`echo $PKG | sed 's/^.*->//'`"
		P="${NAME}-${VERS}.tgz"
		;;
	*)

		# Package name.
		P="${PKG}.tgz"
		;;
	esac

	# Get the package.
	if [ ! -r $P ]
	then

		# Package download log.
		PL="${P}.log"

		# Download package.
		wget -o ${PL} https://ftp.openbsd.org/pub/OpenBSD/7.0/packages/amd64/${P} >/dev/null 2>&1
		if [ $? -ne 0 ]
		then

			# Failed or not there, retry with repository -stable.
			wget -o ${PL} https://ftp.openbsd.org/pub/OpenBSD/7.0/packages-stable/amd64/${P} >/dev/null 2>&1
			if [ $? -ne 0 ]
			then
				echo "$P missing"
			else
				echo "$P downloaded"
			fi
		else
			echo "$P downloaded"
		fi
	else

		# Do not overwrite.
		echo "$PKG already there"
	fi
done

exit 0

