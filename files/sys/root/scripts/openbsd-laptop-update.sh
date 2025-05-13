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

# New files.
NEWFILES=$HOME/openbsd-laptop-update.newfiles

# User and group.
USR=fconagy
GRP=users

# Commit messsage.
COMMIT="$*"

# Create/update save directory under the git directory.
echo "Files directory: $SDIR"
echo "User: $USR"
echo "Group: $GRP"
if [ "$COMMIT" = "" ]
then
	echo "Will not commit"
else
	echo "Commit message: $COMMIT"
fi
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

# Get list of new files.
echo "Get list of new files"
savesys -N -d $SDIR >$NEWFILES

# Save and set ownership.
echo "Save system files"
savesys -o -d $SDIR | tee $LOG
echo "Fix ownership"
chown -R $USR $SDIR
chgrp -R $GRP $SDIR

# Add new files.
echo "Add new files"
cat $NEWFILES | sed 's/^\///' | while read F REST
do
	echo "$F"
	/usr/bin/su - $USR -c "cd $SDIR/sys && git add $F"
	if [ $? -ne 0 ]
	then
		echo "There was an error adding $F"
		exit 1
	fi
done

# Push to git servers.
echo "Push to git servers"
if [ "$COMMIT" = "" ]
then

	# Don't commit.
	/usr/bin/su - $USR -c "cd $LDIR && ssh-git ./git-push.sh; echo 'Waiting'; sleep 10"
else
	/usr/bin/su - $USR -c "cd $LDIR && ssh-git ./git-push.sh $COMMIT; echo 'Waiting'; sleep 10"
fi

# Finish.
exit 0

