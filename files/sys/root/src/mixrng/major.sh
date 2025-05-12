#!/bin/sh

# List device major numbers.

# As they are in MAKEDEV.
cat /dev/MAKEDEV | egrep -e '	M ' | awk '
 {
	if ($3 == "c" || $3 == "b")
	{
		printf "%6d %-32s\n", $4, $2
	}
} ' | sort -n -k 1 | \
awk ' { print $1 } ' | sort -n -u >x.1

# Character and block devices under /dev.
(find /dev -type c -ls;
 find /dev -type b -ls) | awk '
 {
	printf "%6d %-32s\n", $7, $12
} ' | sort -n -k 1 | \
awk ' { print $1 } ' | sort -n -u >x.2

# Show the difference.
echo "===== Device major number differences between MAKEDEV and /dev files"
diff x.1 x.2
rm x.1 x.2

# List them from under /dev.
echo ""
echo "====== Devices under /dev"
(find /dev -type c -ls;
 find /dev -type b -ls) | awk '
 {
	printf "%6d %-32s\n", $7, $12
} ' | sort -u -n -k 1 | sed 's/\/dev\///'

exit 0

