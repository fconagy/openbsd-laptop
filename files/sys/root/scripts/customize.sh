#!/bin/sh

# Customize.

# Packet filter log has wrong permissions.
PFL=/var/log/pflog
echo -n >$PFL
chown _pflogd $PFL
chgrp _pflogd $PFL
chmod o-rwx $PFL
chmod g+rw $PFL

