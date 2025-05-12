#!/bin/sh

# Save original root scripts.

# Quit on error.
set -e

# From directory.
ROOT=/root

# Config files.
SCRIPTS=".Xdefaults .cshrc .cvsrc .login .profile"
for F in $SCRIPTS
do
	cp -p $ROOT/$F .
done

