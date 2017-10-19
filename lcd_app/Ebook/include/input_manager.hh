#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__


typedef struct Input_Operation
{
	const char * c_pcName;
	int (*Input_Init)(void);
	void (*Input_Exit)(void);
	int (*Input_Get_Key)(unsigned int *pdwKey);
	struct Input_Operation *ptNext;
}T_Input_Opr, *PT_Input_Opr;


extern int Input_Opr_Regisiter(PT_Input_Opr ptFontOpr);
extern void Show_Input_Opr(void);
extern PT_Input_Opr Get_Input_Opr(char *pcName);
extern int Input_Opr_Init(void);

#endif

