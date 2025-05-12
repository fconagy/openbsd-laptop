#!/bin/sh

# Connect to home network.

# Interface.
IF=iwm0

# Address.
ADDR=192.168.0.88

# Default gateway. Also DNS.
GW=192.168.0.1

# Start with blank slate.
ifconfig $IF down

# This is when IPV6 still enabled.
#ifconfig $IF -inet6 $ADDR netmask 255.255.255.0 up
#ifconfig lo0 -inet6
#route flush -inet6

# Just configure.
ifconfig $IF $ADDR netmask 255.255.255.0 nwid "caspian" wpakey "ppfgsi67" up
route -q add default $GW
cat <<!EOF >/etc/resolv.conf
family inet4
nameserver 83.255.255.1
nameserver 83.255.255.2
!EOF

