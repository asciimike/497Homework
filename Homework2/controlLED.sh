#!/bin/bash
# From http://wh1t3s.com/2009/05/14/reading-beagleboard-gpio/
# Orginally /usr/bin/readgpio, Modified by Mark A. Yoder 20-Jul-2011
#
# Toggle a GPIO input

if [ $# -lt 2 ]; then
    echo "Usage: $0 <button pin#> <LED pin#>"
    exit 0
fi
BUTTON=$1
LED=$2

cleanup() { # Set button to input, set LED to off
  echo "Cleaning up"
  echo "in" > /sys/class/gpio/gpio${BUTTON}/direction
  echo 0 > /sys/class/gpio/gpio${LED}/value
  echo ""
  exit
}

# Set up the button
if [ ! -e /sys/class/gpio/gpio$BUTTON ]; then
    echo "$BUTTON" > /sys/class/gpio/export
fi
echo "in" > /sys/class/gpio/gpio${BUTTON}/direction

# Set up the LED
if [ ! -e /sys/class/gpio/gpio$LED ]; then
    echo "$LED" > /sys/class/gpio/export
fi
echo "out" > /sys/class/gpio/gpio${LED}/direction

trap cleanup SIGINT # call cleanup on Ctrl-C

# Read forever

while [ "1" = "1" ]; do

  THIS_VALUE=`cat /sys/class/gpio/gpio${BUTTON}/value`
  echo $THIS_VALUE > /sys/class/gpio/gpio${LED}/value

done

cleanup # call the cleanup routine
