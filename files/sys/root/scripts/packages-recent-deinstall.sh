#!/bin/sh

# List packages as installed recently, from system log file.

# Log file.
LOG=packages-delete-deinstall.log

# Get names of recently installed packages from the system log file
# and reverse the lines. Delete.
cat /var/log/messages | grep 'pkg_add: Added' | sed 's/^.*pkg_add: Added //' | \
	nl | sort -nr | cut -f 2- | \
while read PKG REST
do
	pkg_delete $PKG 2>&1 | tee -a $LOG
	if [ $? -ne 0 ]
	then
		echo "==== $PKG delete failed"
		echo "==== $PKG delete failed" >>$LOG
	else
		echo "==== $PKG deleted"
		echo "==== $PKG deleted" >>$LOG
	fi
done

exit 0

