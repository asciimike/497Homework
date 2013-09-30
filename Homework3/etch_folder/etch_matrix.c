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

/*
 * Written by Mike McDonald with code adapted from Mark Yoder.
 * 14-Sept-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio-utils.h"
#include "i2c-dev.h"
#include "i2cbusses.h"

enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NONE
};

typedef struct {
   int x;
   int y;
} point;

// #define DEBUG
#define N 8
#define BICOLOR		// undef if using a single color display

void printGrid(int a[N][N], int isErasing);
point updatePoint(int dir, point pt);
void updateBoardBasedOnCurrentPoint(int grid[N][N], point pt, int isErasing);
void arrayFromMatrix(int grid[N][N], __u16 array[N]);

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
 * I2C Functions
 ***************************************************************/
static int check_funcs(int file) {
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		return -1;
	}

	if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
		fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
		return -1;
	}
	return 0;
}

	// Writes block of data to the display
	static int write_block(int file, __u16 *data) {
	int res;
	#ifdef BICOLOR
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)data);
	return res;
	#else
/*
 * For some reason the single color display is rotated one column, 
 * so pre-unrotate the data.
 */
	int i;
	__u16 block[I2C_SMBUS_BLOCK_MAX];
//	printf("rotating\n");
	for(i=0; i<8; i++) {
		block[i] = (data[i]&0xfe) >> 1 | 
			   (data[i]&0x01) << 7;
	}
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)block);
	return res;
#endif
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[5];
	int nfds = 5;
	int gpio_in_1_fd, gpio_out_1_fd, gpio_in_2_fd, gpio_out_2_fd, gpio_in_3_fd, gpio_out_3_fd, gpio_in_4_fd, gpio_out_4_fd;
	int timeout, rc;
	char buf[MAX_BUF];
	int gpio_in_1, gpio_out_1, gpio_in_2, gpio_out_2, gpio_in_3, gpio_out_3, gpio_in_4, gpio_out_4;
	int len;
	int res, i2cbus, address, file;
	char filename[20];
	int force = 0;

	// run with ./etch_matrix 50 115 51 49 20 48 7 112

	if (argc < 9) {
		printf("Usage: %s <gpio-in> <gpio-out>, ... , ... , ...\n\n");
		printf("Interrupt driven output of four buttons, put in pairs of in/out\n");
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	// Set up I2C Matrix
	i2cbus = lookup_i2c_bus("1");
	printf("i2cbus = %d\n", i2cbus);
	//if (i2cbus < 0)
		//help();

	address = parse_i2c_address("0x70");
	printf("address = 0x%2x\n", address);
	//if (address < 0)
		//help();

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
//	printf("file = %d\n", file);
	if (file < 0
	 || check_funcs(file)
	 || set_slave_addr(file, address, force))
		exit(1);

	// Check the return value on these if there is trouble
	i2c_smbus_write_byte(file, 0x21); // Start oscillator (p10)
	i2c_smbus_write_byte(file, 0x81); // Disp on, blink off (p11)
	i2c_smbus_write_byte(file, 0xe7); // Full brightness (page 15)

	// Initialize GPIO's
	gpio_in_1 = atoi(argv[1]);
	gpio_out_1 = atoi(argv[2]);
	gpio_export(gpio_in_1);
	gpio_export(gpio_out_1);
	gpio_set_dir(gpio_in_1, "in");
	gpio_set_dir(gpio_out_1, "out");
	gpio_set_edge(gpio_in_1, "falling");
	gpio_in_1_fd = gpio_fd_open(gpio_in_1, O_RDONLY);
	gpio_out_1_fd = gpio_fd_open(gpio_out_1, O_RDONLY);

	gpio_in_2 = atoi(argv[3]);
	gpio_out_2 = atoi(argv[4]);
	gpio_export(gpio_in_2);
	gpio_export(gpio_out_2);
	gpio_set_dir(gpio_in_2, "in");
	gpio_set_dir(gpio_out_2, "out");
	gpio_set_edge(gpio_in_2, "falling");
	gpio_in_2_fd = gpio_fd_open(gpio_in_2, O_RDONLY);
	gpio_out_2_fd = gpio_fd_open(gpio_out_2, O_RDONLY);

	gpio_in_3 = atoi(argv[5]);
	gpio_out_3 = atoi(argv[6]);
	gpio_export(gpio_in_3);
	gpio_export(gpio_out_3);
	gpio_set_dir(gpio_in_3, "in");
	gpio_set_dir(gpio_out_3, "out");
	gpio_set_edge(gpio_in_3, "falling");
	gpio_in_3_fd = gpio_fd_open(gpio_in_3, O_RDONLY);
	gpio_out_3_fd = gpio_fd_open(gpio_out_3, O_RDONLY);

	gpio_in_4 = atoi(argv[7]);
	gpio_out_4 = atoi(argv[8]);
	gpio_export(gpio_in_4);
	gpio_export(gpio_out_4);
	gpio_set_dir(gpio_in_4, "in");
	gpio_set_dir(gpio_out_4, "out");
	gpio_set_edge(gpio_in_4, "falling");
	gpio_in_4_fd = gpio_fd_open(gpio_in_4, O_RDONLY);
	gpio_out_4_fd = gpio_fd_open(gpio_out_4, O_RDONLY);

	timeout = POLL_TIMEOUT;

	int grid[N][N];
	int i, j;
	for (i = 0; i < N; ++i)
	{
		for (j = 0; j < N; ++j)
		{
			grid[i][j] = 0;
		}
	}
	
	point currentPoint = {.x = 0, .y = 0};
	int currentDirection = NONE;
	int isErasing = 0;
 
	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_in_1_fd;
		fdset[1].events = POLLPRI;
		
		fdset[2].fd = gpio_in_2_fd;
		fdset[2].events = POLLPRI;

		fdset[3].fd = gpio_in_3_fd;
		fdset[3].events = POLLPRI;
		
		fdset[4].fd = gpio_in_4_fd;
		fdset[4].events = POLLPRI;

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
			}
			//gpio_set_value(gpio_out_1, value);
			currentDirection = UP;
		} else if (fdset[2].revents & POLLPRI) {
			lseek(fdset[2].fd, 0, SEEK_SET);
			len = read(fdset[2].fd, buf, MAX_BUF);
			unsigned int value;
			if (buf[0] == '0') {
				value = 0;
			} else if (buf[0] == '1') {
				value = 1;
			}
			//gpio_set_value(gpio_out_2, value);
			currentDirection = DOWN;
		} else if (fdset[3].revents & POLLPRI) {
			lseek(fdset[3].fd, 0, SEEK_SET);
			len = read(fdset[3].fd, buf, MAX_BUF);
			unsigned int value;
			if (buf[0] == '0') {
				value = 0;
			} else if (buf[0] == '1') {
				value = 1;
			}
			//gpio_set_value(gpio_out_3, value);
			currentDirection = RIGHT;
		} else if (fdset[4].revents & POLLPRI) {
			lseek(fdset[4].fd, 0, SEEK_SET);
			len = read(fdset[4].fd, buf, MAX_BUF);
			unsigned int value;
			if (buf[0] == '0') {
				value = 0;
			} else if (buf[0] == '1') {
				value = 1;
			}
			//gpio_set_value(gpio_out_4, value);
			currentDirection = LEFT;
		} else {
			currentDirection = NONE;
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			//printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
			if (buf[0]=='e') {
				isErasing = !isErasing;
			}
		}

		// Evaluate
		currentPoint = updatePoint(currentDirection, currentPoint);
		#ifdef DEBUG
		printf("After Move: %d, %d\n", currentPoint.x, currentPoint.y);
		#endif
		updateBoardBasedOnCurrentPoint(grid, currentPoint, isErasing);

		// Print the N by N matrix
		printGrid(grid, isErasing);
		// Print N by N matrix to the I2C matrix
		__u16 currentGridToDisplay[N];
		arrayFromMatrix(grid, currentGridToDisplay);
		write_block(file, currentGridToDisplay);
	}

	gpio_fd_close(gpio_in_1_fd);
	gpio_fd_close(gpio_out_1_fd);
	gpio_fd_close(gpio_in_2_fd);
	gpio_fd_close(gpio_out_2_fd);
	gpio_fd_close(gpio_in_3_fd);
	gpio_fd_close(gpio_out_3_fd);
	gpio_fd_close(gpio_in_4_fd);
	gpio_fd_close(gpio_out_4_fd);
	return 0;
}

point updatePoint(int dir, point pt)
{
	point returnPoint;

	#ifdef DEBUG
	printf("Before Move: %d, %d\n", pt.x, pt.y);
	#endif

	switch (dir) {
		case UP:
			#ifdef DEBUG
			printf("Moving up\n");
			#endif
			if (pt.x > 0)
			{
				pt.x--;
			}
			// returnPoint = pt;
			// break;
			return pt;

		case DOWN:
			#ifdef DEBUG
			printf("Moving down\n");
			#endif

			if (pt.x < N-1)
			{
				pt.x++;
			}
			// returnPoint = pt;
			// break;
			return pt;

		case LEFT:
			#ifdef DEBUG
			printf("Moving left\n");
			#endif

			if (pt.y > 0)
			{
				pt.y--;
			}
			// returnPoint = pt;
			// break;
			return pt;

		case RIGHT:
			#ifdef DEBUG
			printf("Moving right\n");
			#endif

			if (pt.y < N-1)
			{
				pt.y++;
			}
			// returnPoint = pt;
			// break;
			return pt;

		case NONE:
			#ifdef DEBUG
			printf("Moving not at all\n");
			#endif

			// returnPoint = pt;
			// break;
			return pt;

		default:
			// Should never get here. Ever.
			// printf("This message should never appear\n");
			break;

		// return returnPoint;
	}
}

void updateBoardBasedOnCurrentPoint(int grid[N][N], point pt, int isErasing)
{
	if (isErasing == 1) {
		grid[pt.x][pt.y] = 0;
	} else {
		grid[pt.x][pt.y] = 1;
	}
}

void arrayFromMatrix(int grid[N][N], __u16 array[N])
{
	__u16 smile_bmp[]={0x3c, 0x42, 0xa9, 0x85, 0x85, 0xa9, 0x42, 0x3c};

	int i, j;
	for (i = 0; i < N; i++) {
		// Iterate over the columns
		__u16 bitPattern = 0x00;
		for (j = 0; j < N; j++) {
			//Pick off the bits in each row of a given column
			int current = grid[i][j];
			current = current << j;
			bitPattern = bitPattern | current;
		}
		array[i] = bitPattern;
	}
}

void printGrid(int a[N][N], int isErasing)
{
	if (isErasing) {
		printf("Erasing on a grid of size %d\n", N);
	} else {
		printf("Writing on a grid of size %d\n", N);
	}
	printf("   ");
	
	int i, j;

	for (i = 0; i < N; ++i)
	{
		printf("%d ",i );
	}
	printf("\n");

	for (i = 0; i < N; ++i)
	{
		printf("%d: ", i);
		for (j = 0; j < N; ++j)
		{
			if (a[i][j] == 1) {
				printf("x ");
			} else {
				printf("  ");
			}
		}
		printf("\n");
	}
	printf("\n");
	printf("\033[%dA",N+3);	// Go back to the top and write over everything
}
