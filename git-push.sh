#!/bin/sh

# Push to GitHub.

# Quit on error.
set -e

# Commit message. Just the argument line.
CMSG="$*"
if [ "$CMSG" = "" ]
then

	# Don't commit.
	:
else

	# File in git repo.
	git commit -a -m "$CMSG"
	if [ $? -ne 0 ]
	then
		echo "Git commit failed"
		exit 1
	fi
fi

# Push.
git push
if [ $? -ne 0 ]
then
	echo "Git push to GitHub failed"
	exit 1
fi
git push nsc
if [ $? -ne 0 ]
then
	echo "Git push to NSC gitlab failed"
	exit 1
fi

# Finish.
exit 0

