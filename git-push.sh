#!/bin/sh

# Push to GitHub.

# Quit on error.
set -e

# Commit message. Just the argument line.
CMSG="$*"
if [ "$CMSG" = "" ]
then
	echo "Specify commit message on the command line"
	exit 1
fi

# File in git repo.
git commit -a -m "$CMSG"

# Push.
git push

# Finish.
exit 0

