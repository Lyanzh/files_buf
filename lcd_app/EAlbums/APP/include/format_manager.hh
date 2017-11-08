#ifndef _FORMAT_MANAGER_H
#define _FORMAT_MANAGER_H

typedef struct Format_Operation
{
	const char * c_pcName;
	int iHeadLen;
	int (*isSupport)(unsigned char *pucBufHead);
	int (*Get_Code)(void);
	struct Format_Operation *ptNext;
} T_Format_Opr, *PT_Format_Opr;

extern int Format_Opr_Regisiter(PT_Format_Opr ptFormatOpr);
extern void Show_Format_Opr(void);
extern PT_Format_Opr Get_Format_Opr(char *pcName);
extern int Format_Opr_Init(void);

#endif


