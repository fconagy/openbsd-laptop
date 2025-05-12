#!/bin/sh

# Remove mixrng messages from system log file.

LOG=/var/log/messages

cat $LOG | egrep -v -e '^mixrng:' | more

