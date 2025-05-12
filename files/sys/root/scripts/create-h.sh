#!/bin/sh

# Create /h directory structure.

set -x

# Quit on error.
set -e

# Home.
H=/h

createdir() {

	# Top.
	if [ ! -d $H ]
	then
		mkdir -p $H
	fi

	# Directory.
	if [ ! -d $1/$2 ]
	then
		mkdir $1/$2
		chmod g+s $1/$2
	fi

	# Link.
	if [ ! -L $H/$2 ]
	then
		ln -s $1/$2 $H/$2
	fi
}

# User space.
createdir /usr/user88 users
createdir /usr/user88 admins
createdir /usr/user88 save
createdir /h/mnt/user90/naxos archive
createdir /h/mnt/user90/naxos firmware
createdir /h/mnt/user90/naxos pkg-cache

