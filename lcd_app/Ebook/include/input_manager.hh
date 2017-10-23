#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__
#include <sys/time.h>

#define INPUT_TYPE_STDIN		0
#define INPUT_TYPE_TOUCHSCREEN	1

#define INPUT_VALUE_UNKNOWN	0
#define INPUT_VALUE_UP		1
#define INPUT_VALUE_DOWN	2
#define INPUT_VALUE_EXIT	3

typedef struct Input_Data {
	int iType;
	int iX;
	int iY;
	char cCode;
} T_Input_Data, *PT_Input_Data;

typedef struct Input_Event {
	struct timeval tTime;
	int iType;
	int iVal;
} T_Input_Event, *PT_Input_Event;

typedef struct Input_Operation
{
	const char * c_pcName;
	int iFd;
	int (*Input_Init)(void);
	void (*Input_Exit)(void);
	int (*Input_Get_Data)(PT_Input_Data ptInputData);
	struct Input_Operation *ptNext;
} T_Input_Opr, *PT_Input_Opr;


extern int Input_Opr_Regisiter(PT_Input_Opr ptFontOpr);
extern void Show_Input_Opr(void);
extern PT_Input_Opr Get_Input_Opr(char *pcName);
extern int All_Input_Device_Init(void);
extern int Input_Get_Key(PT_Input_Event ptInputEvent);
extern int Input_Opr_Init(void);

extern int Stdin_Input_Init(void);

#endif

