#!/bin/sh
# From http://wh1t3s.com/2009/05/14/reading-beagleboard-gpio/
#
# Read an analog input from a specified port, and continuously displays the result to the console
# Created by Mark Yoder, Modified by Mike McDonald 13-Sep-2013

if [ $# -lt 1 ]; then
    echo "Usage: $0 AIN#"
    exit 0
fi

AIN=$1

cleanup() { # echo a newline
  echo ""
  echo "Done"
  exit
}

trap cleanup SIGINT # call cleanup on Ctrl-C

THIS_VALUE=`cat /sys/devices/ocp.2/helper.14/AIN${AIN}`
LAST_VALUE=$THIS_VALUE
echo -ne ${THIS_VALUE}\\r

# Read forever

while [ "1" = "1" ]; do
  # next three lines detect state transition
  if [ "$THIS_VALUE" != "$LAST_VALUE" ]; then
    echo -ne "    "\\r
    echo -ne ${THIS_VALUE}\\r
  fi

  # sleep for a while
  sleep 0.1

  LAST_VALUE=$THIS_VALUE
  THIS_VALUE=`cat /sys/devices/ocp.2/helper.14/AIN${AIN}`

done

cleanup # call the cleanup routine

