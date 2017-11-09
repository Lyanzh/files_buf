#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memwatch.h"

static PT_Page_Opr g_ptPageOprHead;

int Page_Opr_Regisiter(PT_Page_Opr ptPageOpr)
{
	PT_Page_Opr ptPageOprTmp;
	
	if (!g_ptPageOprHead) {
		g_ptPageOprHead = ptPageOpr;
	} else {
		ptPageOprTmp = g_ptPageOprHead;
		while (ptPageOprTmp->ptNext) {
			ptPageOprTmp = ptPageOprTmp->ptNext;
		}
		ptPageOprTmp->ptNext = ptPageOpr;
	}
	ptPageOpr->ptNext = NULL;

	return 0;
}

void Show_Page_Opr(void)
{
	int i = 0;
	PT_Page_Opr ptPageOprTmp = g_ptPageOprHead;
	while (ptPageOprTmp) {
		printf("%d: %s\n", i++, ptPageOprTmp->c_pcName);
		ptPageOprTmp = ptPageOprTmp->ptNext;
	}
}

PT_Page_Opr Get_Page_Opr(char *pcName)
{
	PT_Page_Opr ptPageOprTmp = g_ptPageOprHead;
	while (ptPageOprTmp) {
		if (strcmp(ptPageOprTmp->c_pcName, pcName) == 0) {
			printf("get input %s.\n", pcName);
			return ptPageOprTmp;
		} else {
			ptPageOprTmp = ptPageOprTmp->ptNext;
		}
	}
	printf("Error:can't get input %s.\n", pcName);
	return NULL;
}

int Page_Opr_Init(void)
{
	return 0;
}

