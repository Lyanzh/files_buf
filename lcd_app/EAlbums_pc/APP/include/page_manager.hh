#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include "page_mem.h"
#include "disp_manager.h"
#include "format_manager.h"
#include "input_manager.h"
#include "draw.h"
#include "file.h"
#include "config.h"

typedef struct Page_Operation
{
	const char *c_pcName;
	void (*Run)(void);
	void (*PrepareSelf)(void);
	void (*PrepareNext)(void);
	void (*Get_Input_Event)(void);
	void (*Exit)(void);
	struct Page_Operation *ptNext;
} T_Page_Opr, *PT_Page_Opr;

int Main_Page_Init(void);
int Auto_Page_Init(void);
int Browse_Page_Init(void);
int Setting_Page_Init(void);
int Timer_Page_Init(void);

extern int Page_Opr_Regisiter(PT_Page_Opr ptPageOpr);
extern void Show_Page_Opr(void);
extern PT_Page_Opr Get_Page_Opr(char *pcName);
extern int Page_Opr_Init(void);
extern void Page_Change(char *pcName);

#endif

