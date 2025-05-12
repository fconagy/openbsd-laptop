#!/bin/sh

# Show dropped packets.

tcpdump -n -e -ttt -i pflog0

