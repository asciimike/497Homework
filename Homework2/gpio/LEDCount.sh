#!/bin/bash
# 
#
# Display the number of times a button has been pressed
if [ $# -lt 1 ]; then
    echo "Usage: $0 <gpio pin#>"
    exit 0
fi
BUTTON=$1

# Open the GPIO port
#
echo "$BUTTON" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio${BUTTON}/direction

cleanup() { # Set the GPIO port to 0
  echo "Cleaning up"
  echo $BUTTON > /sys/class/gpio/unexport
  exit
}
trap cleanup SIGINT # call cleanup on Ctrl-C

# Run forever

THIS_VALUE=`cat /sys/class/gpio/gpio${BUTTON}/value`
COUNT=0
while [ "1" = "1" ]; do
  if [ "1" = "$THIS_VALUE" ]; then
     COUNT = 'expr $COUNT + 1'
     echo "Button has been pressed $COUNT times"
     # sleep for a while
     sleep 0.05
  fi
  
done

cleanup # call the cleanup routine

