#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ICON_X(w, t, i)	((Selected_Display()->tDevAttr.dwXres - w) / 2)
#define ICON_Y(h, t, i)	((Selected_Display()->tDevAttr.dwYres - h * t) / (t + 1) * (i + 1) + (h * i))

static T_IconInfo t_TimerPageIcon[] = 
{
	{"icon/time.bmp", 0, 0, 0, 0},
};

static T_PicRegion tPicRegSrc;
static T_PicRegion tPicRegDst;

static void Timer_Page_Prepare(void)
{

}

static void Timer_Page_Run(void)
{
	int i;
	int iIconNum;

	iIconNum = sizeof(t_TimerPageIcon) / sizeof(T_IconInfo);
	for (i = 0; i < iIconNum; i++) {
		Get_Format_Opr("bmp")->Get_Pic_Region(t_TimerPageIcon[i].pcName, &tPicRegSrc);
		tPicRegDst.dwWidth = Selected_Display()->tDevAttr.dwXres / 10;
		tPicRegDst.dwHeight = tPicRegDst.dwWidth;
		t_TimerPageIcon[i].iTopLeftX = ICON_X(tPicRegDst.dwWidth, iIconNum, i);
		t_TimerPageIcon[i].iTopLeftY = ICON_Y(tPicRegDst.dwHeight, iIconNum, i);
		t_TimerPageIcon[i].iBottomRightX = t_TimerPageIcon[i].iTopLeftX + tPicRegDst.dwWidth;
		t_TimerPageIcon[i].iBottomRightY = t_TimerPageIcon[i].iTopLeftY + tPicRegDst.dwHeight;
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, 0.5);
		Lcd_Show_Pic(t_TimerPageIcon[i].iTopLeftX, t_TimerPageIcon[i].iTopLeftY, &tPicRegDst);
		Do_Free(tPicRegSrc.pcData);
		Do_Free(tPicRegDst.pcData);
	}
}

static void Timer_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	while (1) {
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.cCode == 'u') {
			printf("\nplus.\n");
		} else if (tInputEvent.cCode == 'd') {
			printf("\nminer.\n");
		}
	}
}

static T_Page_Opr g_tTimerPageOpr = {
	.c_pcName        = "timerpage",
	.PrepareSelf     = Timer_Page_Prepare,
	.Run		     = Timer_Page_Run,
	.Get_Input_Event = Timer_Page_Get_Input_Event,
};

int Timer_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tTimerPageOpr);
}

