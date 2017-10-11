#include "disp_manager.h"
#include <stdio.h>

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
		ptDispDev->ptNextDev = NULL;
	}

	return 0;
}

int Disp_Init(void)
{
	return Fb_Dev_Init();
}

int Disp_Draw_Bitmap()
{
	
}

