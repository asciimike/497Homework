#!/bin/bash

if [ $# -lt 3 ]; then
    echo "Usage: $0 <i2c bus> <device id> <register>"
    exit 0
fi
BUS=$1
DEVICE=$2
REGISTER=$3

cleanup() {
  echo "Cleaning up"
  echo ""
  exit
}

trap cleanup SIGINT # call cleanup on Ctrl-C

# Read forever
F_HEX=0
while [ "1" = "1" ]; do
C_HEX=`i2cget -y $BUS 0x$DEVICE 0x$REGISTER`
(( F_HEX=C_HEX*2+32 )) # note that F=C*2+32 rather than F=C*1.8+32, so it's going to be off by a bit
printf "Raw hex = %x, decimal = %dC ~ %dF\r" "$C_HEX" "$C_HEX" "$F_HEX"
done

cleanup # call the cleanup routine

