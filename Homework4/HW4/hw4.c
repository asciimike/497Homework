// Homework 4
// Matt Skorina and Michael McDonald

#include "gpio-utils.h"
//#include "analog-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>

void stepcw(void);
void stepccw(void);
void cw_to_ccw(void);
void ccw_to_cw(void);
int read_gpio_value(int pin);
int read_adc_value(int pin);
int step_index = 0;

#define ARRAY_LENGTH 44


const unsigned char step_tbl[8][4]={
{0,0,0,1},
{0,0,1,1},
{0,0,1,0},
{0,1,1,0},
{0,1,0,0},
{1,1,0,0},
{1,0,0,0},
{1,0,0,1}};

/*
const unsigned char step_tbl[4][4]={
{0,0,1,1},
{0,1,1,0},
{1,1,0,0},
{1,0,0,1}};
*/
void search(void);
void track(void);
void write_pins();



//int avg_light[20];
//int max_light = 1023;
int max_light_pos = 0;
int dir = 1;

int start_button = 48; //Right most button 
int stepper_1 = 30;
int stepper_2 = 60;
int stepper_3 = 31;
int stepper_4 = 50;
int left_photo = 3;
int right_photo = 5;

//#define left_photo;

void main(void)
  {     unsigned long int j;
     	unsigned char i;
int test;
	system("sh setup.sh"); //Enable analog inputs
	//test = system("sh get_analog.sh");
//printf("%d",test);

	gpio_export(stepper_1);
	gpio_set_dir(stepper_1, "out");
	gpio_export(stepper_2);
	gpio_set_dir(stepper_2, "out");
	gpio_export(stepper_3);
	gpio_set_dir(stepper_3, "out");
	gpio_export(stepper_4);
	gpio_set_dir(stepper_4, "out");
		
gpio_export(start_button);
gpio_set_dir(start_button, "in");


		while (read_gpio_value(start_button) == 1);
		//for(j=0;j<4000;j++);
//sleep(.01);
		//while (read_gpio_value(start_button) == 1);
		//for(j=0;j<4000;j++);
//sleep(.01);
//fprintf(stdout,"BUTTON");
		search();


	while(1)
      {

		track();
}
   		
}


int read_gpio_value(int pin) {

	char buf[5];
	int fileseeker,len;
	fileseeker = gpio_fd_open(pin,O_RDONLY);
	lseek(fileseeker,0,SEEK_SET);
	len = read(fileseeker,buf,5);
	gpio_fd_close(pin);
	
if (buf[0] == '1') {
//printf("1");
	return 1;

}else {
//printf("0");
return 0;
}
}

int read_adc_value(int pin) {

	char buf[4];
	int fileseeker,len;
	fileseeker = gpio_fd_open(pin,O_RDONLY);
	lseek(fileseeker,0,SEEK_SET);
	len = read(fileseeker,buf,4);
	gpio_fd_close(pin);

	//return atoi(buf[0])*1000+atoi(buf[1])*100+atoi(buf[2])*10+atoi(buf[3]);
return 0;

}


void stepcw()
{	unsigned int count;

//int pin_array = step_tbl[step_index];
/*
printf("%d",pin_array[0]);
printf("%d",pin_array[1]);
printf("%d",pin_array[2]);
printf("%d\n",pin_array[3]);
*/
write_pins();
//usleep(100000);
usleep(50000);
	step_index++;
	if (step_index == 8) step_index = 0;	//Wrap when index gets up to 4


//pin_array = {0,1,1,0};
//write_pins(&pin_array);
//sleep(2);
//delay(.05);
}
void stepccw()
{	unsigned int count;
	//PORTD = step_tbl[step_index];			//Send out the next step code from table.
	write_pins();
//usleep(100000);
	usleep(50000);
	step_index--;
	if (step_index == -1) step_index = 7;	//Wrap when index gets up to 4						//Step downward through the stepping seq table						
	//if (step_index == -1) step_index = 3;	//Wrap when index gets down to -1
	//for(count=0;count<15000;count++);		//This delay loop sets the step time
}

void write_pins() {
	gpio_set_value(stepper_1, step_tbl[step_index][0]);
	gpio_set_value(stepper_2, step_tbl[step_index][1]);
	gpio_set_value(stepper_3, step_tbl[step_index][2]);
	gpio_set_value(stepper_4, step_tbl[step_index][3]);
}
void cw_to_ccw()							//Index pointer needs to be backed up
{	step_index--;							//When changing direction cw to ccw.
	if (step_index==-1) step_index = 7;
}
void ccw_to_cw()							//Index pointer needs to be moved forward
{	step_index++;							//when changing direction ccw to cw
	if (step_index==8) step_index = 0;
}

void search() {
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int *left_value;
	unsigned int *right_value;
	unsigned int left_values[ARRAY_LENGTH];
unsigned int right_values[ARRAY_LENGTH];
unsigned int max_light = 99999999;


	// First value read is always crappy, so read one and through it away
	ain_get_value(left_photo,&left_value);
	ain_get_value(right_photo,&right_value);
	for (i=0;i<ARRAY_LENGTH;i++) {
		ain_get_value(left_photo,&left_value);
		ain_get_value(right_photo,&right_value);
		
left_values[i] = left_value;
right_values[i] = right_value;
		//printf("Left: %d, Right: %d\n",left_value,right_value);

		//avg_light[i] = (*left_value + *right_value) / 2;
		stepcw();
	}
	for (j=0;j<ARRAY_LENGTH;j++) {
		if (left_values[j] < max_light) {
			max_light = left_values[j];
			max_light_pos = j;
		}
	}
	
	sleep(1);

	//cw_to_ccw();
	for (k=0;k<44;k++) {
		ain_get_value(left_photo,&left_value);
		//usleep(1000);
		ain_get_value(right_photo,&right_value);
		//printf("Left: %d, Right: %d\n",left_value,right_value);
		stepccw();
	}
}

void track() {
unsigned int *left_value;
	unsigned int *right_value;
ain_get_value(left_photo,&left_value);
	ain_get_value(right_photo,&right_value);
	int left =0; //= ADC_convert(0);
	int right =0; //= ADC_convert(1);

		if (left_value > right_value) {
			if (dir == 0 ) {
				cw_to_ccw();
				dir =1;
			}
			stepccw();
		} else {
			if (dir == 1 ) {
				ccw_to_cw();
				dir =0;
			}
			stepcw();
		}	
}

