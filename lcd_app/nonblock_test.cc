#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#define NB_ENABLE 1
#define NB_DISABLE 0

int kbhit()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state)
{
	struct termios ttystate;
 
	//get the terminal state
	tcgetattr(STDIN_FILENO, &ttystate);
 
	if (state == NB_ENABLE) {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
	} else if (state == NB_DISABLE) {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int main()
{
	char c;
	int i = 0;
 
	nonblock(NB_ENABLE);
	while (1) {
		usleep(100);
		i = kbhit();
		if (i != 0) {
			c = fgetc(stdin);
			printf("you hit %c. \n", c);
		}
	}
	nonblock(NB_DISABLE);

	return 0;
}

