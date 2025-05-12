#!/bin/sh

# Set up printing.

# In /etc/printcap.
# Our HP LaserJet 4 printer, connected via special parallel USB interface.
#lp|hplaser:\
#	:lp=/dev/ulpt0:sd=/var/spool/output/hplaser:lf=/var/log/lpd-errs:

# Quit on error.

# Create print spool directory.
mkdir /var/spool/output/hplaser
chown -R root:print /var/spool/output/hplaser
chmod 770 /var/spool/output/hplaser

