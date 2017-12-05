#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "memwatch.h"

static PT_Page_Opr g_ptPrePageOpr;
static PT_Page_Opr g_ptCurPageOpr;

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
			//printf("get page %s.\n", pcName);
			return ptPageOprTmp;
		} else {
			ptPageOprTmp = ptPageOprTmp->ptNext;
		}
	}
	printf("Error:can't get page %s.\n", pcName);
	return NULL;
}

int Page_Opr_Init(void)
{
	int iRet;
	int iErr;

	iRet = Main_Page_Init();
	if (0 == iRet) {
		iErr = 0;
	}

	iRet = Auto_Page_Init();
	if (0 == iRet) {
		iErr = 0;
	}

	iRet = Browse_Page_Init();
	if (0 == iRet) {
		iErr = 0;
	}

	iRet = Setting_Page_Init();
	if (0 == iRet) {
		iErr = 0;
	}

	iRet = Timer_Page_Init();
	if (0 == iRet) {
		iErr = 0;
	}
	return iErr;
}

void Page_Change(char *pcName)
{
	PT_Page_Opr g_ptPageOprTmp;
	g_ptPageOprTmp = Get_Page_Opr(pcName);
	/* �����һҳ���ڣ���ɾ����ǰҳ�����ݣ���ʾ��һҳ */
	if (g_ptPageOprTmp) {
		/* ��ʼʱg_ptCurPageOprΪ�գ�������Ҫɾ����ǰҳ������ */
		if (g_ptCurPageOpr) {
			g_ptCurPageOpr->Exit();
		}
		g_ptPrePageOpr = g_ptCurPageOpr;
		g_ptCurPageOpr = g_ptPageOprTmp;
		g_ptCurPageOpr->Run();
		g_ptCurPageOpr->PrepareNext();
		g_ptCurPageOpr->Get_Input_Event();
	}
}

