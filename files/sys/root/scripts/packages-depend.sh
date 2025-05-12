#!/bin/sh

# Log file and list file.
LOG=packages-depend.log
LIST=packages-depend.list
rm -f $LIST

# List dependencies.

depends() {
	if [ "$1" != "" ]
	then
		pkg_info -f $1 | grep '^@depend' | sed 's/^.*://'
	fi
}

# Required by a package.

required() {
	if [ "$1" != "" ]
	then
		depends $1 | while read REQ
		do
			if [ "$REQ" != "" ]
			then
				if [ -d /var/db/pkg/$REQ ]
				then
					:
				else
					echo "$REQ"
					printf "%s %s\n" "$1" "$REQ" >>$LIST
				fi
			fi
		done
	fi
}

# Print dependencies indented.

printdep()
{
	INDENT="`expr $1 \* 2`"
	if [ "$2" != "" ]
	then
		printf "%${INDENT}s%s\n" "" "$2"
	fi
}

# Trying to do recursion in not a good way.
for P0 in $*
do
	printdep 0 "$P0"
	required "$P0" | \
		while read P1
	do
		printdep 1 "$P1"
		required "$P1" | \
			while read P2
		do
			printdep 2 "$P2"
			required "$P2" | \
				while read P3
			do
				printdep 3 "$P3"
				required "$P3" | \
					while read P4
				do
					printdep 4 "$P4"
					required "$P4" | \
						while read P5
					do
						printdep 5 "$P5"
						required "$P5" | \
							while read P6
						do
							printdep 6 "$P6"
						done
					done
				done
			done
		done
	done
done >$LOG

# Print curated list.
cat $LIST | awk ' { print $2 } ' | sort -u

# Finish.
rm $LIST
rm $LOG
exit 0

