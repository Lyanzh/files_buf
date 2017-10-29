#include "input_manager.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#include "memwatch.h"

#define NB_ENABLE  1
#define NB_DISABLE 0

#if 0
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
#endif

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

static int Stdin_Get_Data(PT_Input_Data ptInputData)
{
	char cGetChar;
	//int iRet = 0;
	
	//iRet = kbhit();
	//if (iRet != 0) {
		cGetChar = fgetc(stdin);/* fgetc»á×èÈû */
		//printf("you hit %c.\n", cGetChar);
		ptInputData->iType = INPUT_TYPE_STDIN;
		ptInputData->cCode = cGetChar;
		return 1;
	//}

	//return 0;
}

static T_Input_Opr g_tStdioOpr = {
	.c_pcName   = "stdin",
	//.iFd        = STDIN_FILENO,
	.Input_Init = Stdin_Init,
	.Input_Exit = Stdin_Exit,
	.Input_Get_Data = Stdin_Get_Data,
};

int Stdin_Input_Init(void)
{
	return Input_Opr_Regisiter(&g_tStdioOpr);
}

