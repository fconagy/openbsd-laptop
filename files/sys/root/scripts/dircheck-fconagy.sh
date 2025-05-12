#!/bin/sh

# Check and compare two directories.

#dirsync -v -t -c /mnt/save00/save/naxos/user00/users/fconagy /h/users/fconagy | egrep -v -e '^\.' | more
dirsync -v -t -c /mnt/save00/save/naxos/user00/users/fconagy /h/users/fconagy | egrep -v -e '^\.' | egrep -v -e '\.cache\/|\.config\/' | more

