#!/bin/sh
# From http://wh1t3s.com/2009/05/14/reading-beagleboard-gpio/
#
# Print out the number of times a button has been pressed
# Adapted from readgpio.sh by Mark Yoder
# Created by Mike McDonald 12-Sept-2013

GPIO=$1

cleanup() { # Release the GPIO port
  echo $GPIO > /sys/class/gpio/unexport
  echo ""
  echo ""
  exit
}

# Open the GPIO port
#
echo "$GPIO" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio${GPIO}/direction

trap cleanup SIGINT # call cleanup on Ctrl-C

# Read forever
count=0
while [ "1" = "1" ]; do
  # Get a new value for the button press
  THIS_VALUE=`cat /sys/class/gpio/gpio${GPIO}/value`

  # Check if button is pressed
  if [ "1" = "$THIS_VALUE" ]; then
    # Increment the count and print out the new value
    (( count += 1 ))
    echo "Button pressed $count times"
    
    # Catch and hold the pressed button 
    while [ "1" = "$THIS_VALUE" ]; do
      THIS_VALUE=`cat /sys/class/gpio/gpio${GPIO}/value`
    done
  fi
done

cleanup # call the cleanup routine
