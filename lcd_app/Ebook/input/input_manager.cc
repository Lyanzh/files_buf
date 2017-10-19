#include "input_manager.h"
#include <stdio.h>
#include <string.h>

#include "memwatch.h"

static PT_Input_Opr g_ptInputOprHead;

int Input_Opr_Regisiter(PT_Input_Opr ptInputOpr)
{
	PT_Input_Opr ptInputOprTmp;
	
	if (!g_ptInputOprHead) {
		g_ptInputOprHead = ptInputOpr;
	} else {
		ptInputOprTmp = g_ptInputOprHead;
		while (ptInputOprTmp->ptNext) {
			ptInputOprTmp = ptInputOprTmp->ptNext;
		}
		ptInputOprTmp->ptNext = ptInputOpr;
	}
	ptInputOpr->ptNext = NULL;

	return 0;
}

void Show_Input_Opr(void)
{
	int i = 0;
	PT_Input_Opr ptInputOprTmp = g_ptInputOprHead;
	while (ptInputOprTmp) {
		printf("%d: %s\n", i++, ptInputOprTmp->c_pcName);
		ptInputOprTmp = ptInputOprTmp->ptNext;
	}
}

PT_Input_Opr Get_Input_Opr(char *pcName)
{
	PT_Input_Opr ptInputOprTmp = g_ptInputOprHead;
	while (ptInputOprTmp) {
		if (strcmp(ptInputOprTmp->c_pcName, pcName) == 0) {
			printf("get input %s.\n", pcName);
			return ptInputOprTmp;
		} else {
			ptInputOprTmp = ptInputOprTmp->ptNext;
		}
	}
	printf("Error:can't get input %s.\n", pcName);
	return NULL;
}

int Input_Opr_Init(void)
{
	int iError;
	iError =  Stdin_Input_Init();
	if (iError) {
		printf("Error:ASCII encoding init fail.\n");
		return -1;
	}
	return 0;
}

