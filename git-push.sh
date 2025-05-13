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
fi

# Push.
git push

# Finish.
exit 0

