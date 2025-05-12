#!/bin/sh

# Start mouse, right button pastes.
wsmoused -M 2=3 -M 3=2

# Try to make mouse slower.
wsconsctl mouse.param=0:770,1:600
wsconsctl mouse2.param=0:401,1:401

