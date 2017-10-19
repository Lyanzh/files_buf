#include "input_manager.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#include "memwatch.h"

#define NB_ENABLE 1
#define NB_DISABLE 0

static int kbhit()
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

static void nonblock(int state)
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

static int Stdin_Init(void)
{
	nonblock(NB_ENABLE);
	return 0;
}

static void Stdin_Exit(void)
{
	nonblock(NB_DISABLE);
}

static int Stdin_Get_Key(unsigned int *pdwKey)
{
	char cGetChar;
	int iRet = 0;
 
	usleep(100);
	iRet = kbhit();
	if (iRet != 0) {
		cGetChar = fgetc(stdin);
		printf("you hit %c.\n", cGetChar);
		*pdwKey = cGetChar;
		return 1;
	}

	return 0;
}

static T_Input_Opr g_tInputOpr = {
	.c_pcName   = "stdin",
	.Input_Init = Stdin_Init,
	.Input_Exit = Stdin_Exit,
	.Input_Get_Key = Stdin_Get_Key,
};

int Stdin_Input_Init(void)
{
	return Input_Opr_Regisiter(&g_tInputOpr);
}

