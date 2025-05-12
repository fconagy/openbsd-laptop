#!/bin/sh

# Update files under the openbsd-laptop git.

# Log file.
LOG=openbsd-laptop-update.log

# Openbsd laptop documentation directory.
LDIR=/h/users/fconagy/openbsd-laptop
if [ ! -d $LDIR ]
then
	echo "Cannot find $LDIR"
	exit 1
fi

# Directory for savesys files.
SDIR=$LDIR/files
if [ ! -d $SDIR ]
then
	echo "Cannot find $SDIR - create it"
	exit 1
fi

# User and group.
USR=fconagy
GRP=users

# Commit messsage.
COMMIT="$*"

# Create/update save directory under the git directory.
echo "Files directory: $SDIR"
echo "User: $USR"
echo "Group: $GRP"
echo "Commit message: $COMMIT"
echo -n "Continue [y/N]: "
read ANS
case $ANS in
[yY]|[yY][eE][sS])
	;;
*)
	echo "Execution canceled"
	exit 1
	;;
esac

# Quit on error.
set -e

# Save and set ownership.
savesys -v -o -d $SDIR | tee $LOG
chown -R $USR $SDIR
chgrp -R $GRP $SDIR

# Push to git servers.
#/usr/bin/su - $USR -c "cd $SDIR && ssh-git && ./git-push.sh $COMMIT"

# Finish.
exit 0

