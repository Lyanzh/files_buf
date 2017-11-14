#include "format_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memwatch.h"

static PT_Format_Opr g_ptFormatOprHead;

int Format_Opr_Regisiter(PT_Format_Opr ptFormatOpr)
{
	PT_Format_Opr ptFormatOprTmp;
	
	if (!g_ptFormatOprHead) {
		g_ptFormatOprHead = ptFormatOpr;
	} else {
		ptFormatOprTmp = g_ptFormatOprHead;
		while (ptFormatOprTmp->ptNext) {
			ptFormatOprTmp = ptFormatOprTmp->ptNext;
		}
		ptFormatOprTmp->ptNext = ptFormatOpr;
	}
	ptFormatOpr->ptNext = NULL;

	return 0;
}

void Show_Format_Opr(void)
{
	int i = 0;
	PT_Format_Opr ptFormatOprTmp = g_ptFormatOprHead;
	while (ptFormatOprTmp) {
		printf("%d: %s\n", i++, ptFormatOprTmp->c_pcName);
		ptFormatOprTmp = ptFormatOprTmp->ptNext;
	}
}

PT_Format_Opr Get_Format_Opr(char *pcName)
{
	PT_Format_Opr ptFormatOprTmp = g_ptFormatOprHead;
	while (ptFormatOprTmp) {
		if (strcmp(ptFormatOprTmp->c_pcName, pcName) == 0) {
			printf("get input %s.\n", pcName);
			return ptFormatOprTmp;
		} else {
			ptFormatOprTmp = ptFormatOprTmp->ptNext;
		}
	}
	printf("Error:can't get input %s.\n", pcName);
	return NULL;
}

int Format_Opr_Init(void)
{
	return 0;
}


