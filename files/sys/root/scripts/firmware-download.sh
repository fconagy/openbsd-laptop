#!/bin/sh

# Download firmware.

set -e
cd firmware
wget --no-check-certificate -r -l 1 -N http://firmware.openbsd.org/firmware/7.4/

