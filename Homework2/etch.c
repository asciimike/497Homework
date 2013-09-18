#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

// #define DEBUG
#define N 8

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

char getch();
void printGrid(int a[N][N], int isErasing);
point updatePoint(int dir, point pt);
void updateBoardBasedOnCurrentPoint(int grid[N][N], point pt, int isErasing);

int main(int argc, char **argv)
{
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

	while(1){
		// Read
		char data = getch();
    	if (data == 'w') {
    		// Up
    		currentDirection = UP;
    	} else if (data == 's') {
    		// Down
    		currentDirection = DOWN;
    	} else if (data == 'd') {
    		// Right
    		currentDirection = RIGHT;
    	} else if (data == 'a') {
    		// Left
    		currentDirection = LEFT;
    	} else if (data == 'e'){
    		// Toggle erasing
    		if (isErasing)
    		{
    			isErasing = 0;
    		} else {
    			isErasing = 1;
    		}
    		currentDirection = NONE;
    	} else {
    		//Incorrect input here.
    		currentDirection = NONE;
    	}

		// Evaluate
		currentPoint = updatePoint(currentDirection, currentPoint);
		#ifdef DEBUG
		printf("After Move: %d, %d\n", currentPoint.x, currentPoint.y);
		#endif
		updateBoardBasedOnCurrentPoint(grid, currentPoint, isErasing);

		// Print the N by N matrix
		printGrid(grid, isErasing);
	}
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

// Thank you stackoverflow: http://stackoverflow.com/questions/421860/c-c-capture-characters-from-standard-input-without-waiting-for-enter-to-be-pr
// Pulls in characters from the console without the need for a return
char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}