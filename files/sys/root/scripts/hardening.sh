#!/bin/sh

# Hardening.

# Stop unwanted daemons.

unwanted() {
	for D in $*
	do
		echo $D
		rcctl stop $D
		rcctl disable $D

		# For example ntp would not die.
		for PID in `ps -axu | grep $D | grep -v grep | awk ' { print $2 } '`
		do
			kill $PID
			sleep 1
			kill -9 $PID
		done
	done
}

# Unwanteds.
unwanted dhcpleased slaacd smtpd ntpd resolvd

# Check.
netstat -an | egrep -e '^tcp|^udp'

