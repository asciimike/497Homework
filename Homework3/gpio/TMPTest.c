/* Copyright (c) 2011, RidgeRun
 * All rights reserved.
 *
From https://www.ridgerun.com/developer/wiki/index.php/Gpio-int-test.c

 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the RidgeRun.
 * 4. Neither the name of the RidgeRun nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY RIDGERUN ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL RIDGERUN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
 * Constants
 ****************************************************************/
 
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

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
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[3];
	int nfds = 3;
	int gpio_in_1_fd, gpio_out_1_fd, gpio_in_2_fd, gpio_out_2_fd;
	int timeout, rc;
	char buf[MAX_BUF];
	int gpio_in_1, gpio_out_1, gpio_in_2, gpio_out_2;
	int len;

	// run with ./TMPTest 50 115 51 49

	if (argc < 5) {
		printf("Usage: %s <gpio-in> <gpio-out>, ... \n\n");
		exit(-1);
	}

	//Set up the TMP 101 sensors
	char firstSensorString[MAX_BUF];
	char secondSensorString[MAX_BUF];
	//These values shouldn't be hard coded, and I feel bad about it.
	sprintf(firstSensorString, "sh setTMP101limits.sh 1 48 19 1b");	
	sprintf(secondSensorString, "sh setTMP101limits.sh 1 4a 19 1b");
	system(firstSensorString);
	system(secondSensorString);
	puts(firstSensorString);
	puts(secondSensorString);

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	gpio_in_1 = atoi(argv[1]);
	gpio_out_1 = atoi(argv[2]);
	gpio_export(gpio_in_1);
	gpio_export(gpio_out_1);
	gpio_set_dir(gpio_in_1, "in");
	gpio_set_dir(gpio_out_1, "out");
	gpio_set_edge(gpio_in_1, "both");
	gpio_in_1_fd = gpio_fd_open(gpio_in_1, O_RDONLY);
	gpio_out_1_fd = gpio_fd_open(gpio_out_1, O_RDONLY);

	gpio_in_2 = atoi(argv[3]);
	gpio_out_2 = atoi(argv[4]);
	gpio_export(gpio_in_2);
	gpio_export(gpio_out_2);
	gpio_set_dir(gpio_in_2, "in");
	gpio_set_dir(gpio_out_2, "out");
	gpio_set_edge(gpio_in_2, "both");
	gpio_in_2_fd = gpio_fd_open(gpio_in_2, O_RDONLY);
	gpio_out_2_fd = gpio_fd_open(gpio_out_2, O_RDONLY);

	timeout = POLL_TIMEOUT;
 
	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_in_1_fd;
		fdset[1].events = POLLPRI;
		
		fdset[2].fd = gpio_in_2_fd;
		fdset[2].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
            
		if (fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET);
			len = read(fdset[1].fd, buf, MAX_BUF);
			unsigned int value;
			if (buf[0] == '0') {
				value = 0;
			} else if (buf[0] == '1') {
				value = 1;
				//Do an i2cget here to snag the temperature, then convert to F
			}
			gpio_set_value(gpio_out_1, value);
		} else if (fdset[2].revents & POLLPRI) {
			lseek(fdset[2].fd, 0, SEEK_SET);
			len = read(fdset[2].fd, buf, MAX_BUF);
			unsigned int value;
			if (buf[0] == '0') {
				value = 0;
			} else if (buf[0] == '1') {
				value = 1;
				//Do an i2cget here to snag the temperature, then convert to F
			}
			gpio_set_value(gpio_out_2, value);
		}

		fflush(stdout);
	}

	gpio_fd_close(gpio_in_1_fd);
	gpio_fd_close(gpio_out_1_fd);
	gpio_fd_close(gpio_in_2_fd);
	gpio_fd_close(gpio_out_2_fd);

	return 0;
}
