#include "disp_manager.h"
#include <stdio.h>
#include <string.h>

#include "memwatch.h"

static PT_Disp_Opr g_ptDispOprHead;
static PT_Disp_Opr g_ptDispOprSelected;

int Disp_Opr_Regisiter(PT_Disp_Opr ptDispOpr)
{
	PT_Disp_Opr ptDispOprTmp;
	
	if (!g_ptDispOprHead) {
		g_ptDispOprHead = ptDispOpr;
	} else {
		ptDispOprTmp = g_ptDispOprHead;
		while (ptDispOprTmp->ptNext) {
			ptDispOprTmp = ptDispOprTmp->ptNext;
		}
		ptDispOprTmp->ptNext = ptDispOpr;
	}
	ptDispOpr->ptNext = NULL;

	return 0;
}

void Show_Disp_Opr(void)
{
	int i = 0;
	PT_Disp_Opr ptDispOprTmp = g_ptDispOprHead;
	while (ptDispOprTmp) {
		printf("%d: %s\n", i++, ptDispOprTmp->pcName);
		ptDispOprTmp = ptDispOprTmp->ptNext;
	}
}

PT_Disp_Opr Get_Disp_Opr(char *pcName)
{
	PT_Disp_Opr ptDispOprTmp = g_ptDispOprHead;
	while (ptDispOprTmp) {
		if (strcmp(ptDispOprTmp->pcName, pcName) == 0)
			return ptDispOprTmp;
		else
			ptDispOprTmp = ptDispOprTmp->ptNext;
	}
	return NULL;
}

int Disp_Opr_Init(void)
{
	int iRet;
	int iError;
	
	iRet = PC_Disp_Dev_Init();
	if (iRet == 0)
		iError = 0;

	iRet = Fb_Dev_Init();
	if (iRet == 0)
		iError = 0;

	return iError;
}

int Select_And_Init_Display(char *pcName)
{
	int iError;
	g_ptDispOprSelected = Get_Disp_Opr(pcName);
	if (!g_ptDispOprSelected) {
		printf("Error:can not get display operation for %s.", pcName);
		return -1;
	}

	iError = g_ptDispOprSelected->Init();
	return iError;
}

PT_Disp_Opr Selected_Display(void)
{
	return g_ptDispOprSelected;
}

