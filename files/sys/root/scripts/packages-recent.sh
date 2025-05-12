#!/bin/sh

# List packages as installed recently, from system log file.

cat /var/log/messages | grep 'pkg_add: Added' | sed 's/^.*pkg_add: Added //'

