#!/bin/bash

if [ $# -lt 4 ]; then
    echo "Usage: $0 <i2c bus> <device id> <low value> <high value>"
    exit 0
fi
BUS=$1
DEVICE=$2
LOW_REGISTER=$3
HIGH_REGISTER=$4

cleanup() {
  exit
}

trap cleanup SIGINT # call cleanup on Ctrl-C

#set up the registers properly
`i2cset -y $BUS 0x$DEVICE 1 0x04`	#set up the proper interrupt (high on interrupt)
`i2cset -y $BUS 0x$DEVICE 2 0x$LOW_REGISTER`	#set the lower limit
`i2cset -y $BUS 0x$DEVICE 3 0x$HIGH_REGISTER`	#set the upper limit

cleanup # call the cleanup routine

