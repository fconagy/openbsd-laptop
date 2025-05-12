#!/bin/sh

# Connect in the office.

# Quit on error.
set -e

# Interface.
IF="$1"
if [ "$1" = "" ]
then
	echo "Specify interface (ure0, axe0)"
	exit 1
fi

# Start with blank slate.
ifconfig $IF down
pf-restart

# When IPV6 still enabled get rid of it.
# Note that route flush returns 1.
#ifconfig lo0 -inet6 || true
#route flush -inet6 || true

# Configure.
#ifconfig $IF -inet6 130.236.101.208 netmask 255.255.255.192 up
#route -q add default 130.236.101.254

# Configure without IPV6.
ifconfig $IF 130.236.101.208 netmask 255.255.255.192 up
route -q add default 130.236.101.254

# Name server.
cat <<!EOF >/etc/resolv.conf
family inet4
nameserver 130.236.101.9
nameserver 130.236.101.73
!EOF

exit 0

