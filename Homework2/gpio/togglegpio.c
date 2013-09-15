// Blink pin 60 at 1 Hz
//
//Created by Dingo_aus, 7 January 2009
//email: dingo_aus [at] internode <dot> on /dot/ net
// From http://www.avrfreaks.net/wiki/index.php/Documentation:Linux/GPIO#gpio_framework
//
//Created in AVR32 Studio (version 2.0.2) running on Ubuntu 8.04
// Modified by Mark A. Yoder, 21-July-2011
// Modified by Mark A. Yoder 30-May-2013
// Modified by Mike McDonald 15-Sept-2013

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio-utils.h"

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf("\nCtrl-C pressed, cleaning up and exiting..\n");
	keepgoing = 0;
}

int main(int argc, char** argv)
{
	//create a variable to store whether we are sending a '1' or a '0'
	char set_value[5]; 
	//Integer to keep track of whether we want on or off
	int toggle = 0;
	int onTime;	// Time in micro sec to keep the signal on/off
	int offTime;
	unsigned int gpio;
	int gpio_fd;

	if (argc < 4) {
		printf("%s <pin #> <on time> <off time>\n",argv[0]);
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	gpio = atoi(argv[1]);
	onTime = atoi(argv[2]);
	offTime = atoi(argv[3]);

	/*
	printf("**********************************\n"
		"*  Welcome to PIN Blink program  *\n"
		"*  ....blinking gpio 60          *\n"
		"*  ....period of %d us.........*\n"
		"**********************************\n", 2*onOffTime);
	*/

	//Using sysfs we need to write the gpio number to /sys/class/gpio/export
	//This will create the folder /sys/class/gpio/gpioXX
	gpio_export(gpio);

	printf("...export file accessed, new pin now accessible\n");
	
	//SET DIRECTION
	gpio_set_dir(gpio, "out");
	printf("...direction set to output\n");
			
	gpio_fd = gpio_fd_open(gpio, O_RDONLY);

	//Run an infinite loop - will require Ctrl-C to exit this program
	while(keepgoing)
	{
		toggle = !toggle;
		gpio_set_value(gpio, toggle);
		
		if (toggle) {
			usleep(onTime);
		} else {
			usleep(offTime);
		}

	}
	gpio_fd_close(gpio_fd);
	return 0;
}
