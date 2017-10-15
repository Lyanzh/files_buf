#include "disp_manager.h"
#include <stdio.h>
#include <string.h>

#include "memwatch.h"

static PT_DispDev g_ptDispDevHead;

int Disp_Dev_Regisiter(PT_DispDev ptDispDev)
{
	PT_DispDev ptDispDevTmp;
	
	if (!g_ptDispDevHead) {
		g_ptDispDevHead = ptDispDev;
	} else {
		ptDispDevTmp = g_ptDispDevHead;
		while (ptDispDevTmp->ptNextDev) {
			ptDispDevTmp = ptDispDevTmp->ptNextDev;
		}
		ptDispDevTmp->ptNextDev = ptDispDev;
	}
	ptDispDev->ptNextDev = NULL;

	return 0;
}

void Show_Disp_Opr(void)
{
	int i = 0;
	PT_DispDev ptDispDevTmp = g_ptDispDevHead;
	while (ptDispDevTmp) {
		printf("%d: %s\n", i++, ptDispDevTmp->c_pDevName);
		ptDispDevTmp = ptDispDevTmp->ptNextDev;
	}
}

PT_DispDev Get_Disp_Opr(char *pcName)
{
	PT_DispDev ptDispDevTmp = g_ptDispDevHead;
	while (ptDispDevTmp) {
		if (strcmp(ptDispDevTmp->c_pDevName, pcName) == 0)
			return ptDispDevTmp;
		else
			ptDispDevTmp = ptDispDevTmp->ptNextDev;
	}
	return NULL;
}

int Disp_Opr_Init(void)
{
	return Fb_Dev_Init();
}

int Disp_Draw_Bitmap(void)
{
	return 0;
}

