#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

typedef struct Page_Operation
{
	const char * c_pcName;
	void (*Run)(void);
	void (*Prepare)(void);
	void (*Get_Input_Event)(void);
	struct Page_Operation *ptNext;
} T_Page_Opr, *PT_Page_Opr;

extern int Page_Opr_Regisiter(PT_Page_Opr ptPageOpr);
extern void Show_Page_Opr(void);
extern PT_Page_Opr Get_Page_Opr(char *pcName);
extern int Page_Opr_Init(void);

#endif

