#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>

#define ICON_X(w, t, i)	((Selected_Display()->tDevAttr.dwXres - w) / 2)
#define ICON_Y(h, t, i)	((Selected_Display()->tDevAttr.dwYres - h * t) / (t + 1) * (i + 1) + (h * i))

static T_IconInfo t_MainPageIcon[] = 
{
	{"icon/browse_mode.bmp", 0, 0, 0, 0},
	{"icon/continue_mode.bmp", 0, 0, 0, 0},
	{"icon/setting.bmp", 0, 0, 0, 0},
};

static T_PicRegion tPicRegSrc;
static T_PicRegion tPicRegDst;

static void Main_Page_Prepare(void)
{
}

static void Main_Page_Run(void)
{
	int i;
	int iIconNum;

	iIconNum = sizeof(t_MainPageIcon) / sizeof(T_IconInfo);
	for (i = 0; i < iIconNum; i++) {
		Get_Format_Opr("bmp")->Get_Pic_Region(t_MainPageIcon[i].pcName, &tPicRegSrc);
		tPicRegDst.dwWidth = Selected_Display()->tDevAttr.dwXres / 5;
		tPicRegDst.dwHeight = tPicRegDst.dwWidth / 2;
		t_MainPageIcon[i].iTopLeftX = ICON_X(tPicRegDst.dwWidth, iIconNum, i);
		t_MainPageIcon[i].iTopLeftY = ICON_Y(tPicRegDst.dwHeight, iIconNum, i);
		t_MainPageIcon[i].iBottomRightX = t_MainPageIcon[i].iTopLeftX + tPicRegDst.dwWidth;
		t_MainPageIcon[i].iBottomRightY = t_MainPageIcon[i].iTopLeftY + tPicRegDst.dwHeight;
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, 0);
		Fb_Lcd_Show_Pic(t_MainPageIcon[i].iTopLeftX, t_MainPageIcon[i].iTopLeftY, &tPicRegDst);
	}
}

static void Main_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	Input_Get_Key(&tInputEvent);
	if (tInputEvent.cCode == 'b') {
		printf("\nshow browse page.\n");
		//Socket_Send("show next page.\n");
	} else if (tInputEvent.cCode == 'c') {
		printf("\nshow continue mode page.\n");
	} else if (tInputEvent.cCode == 's') {
		printf("\nshow setting page.\n");
	}
}

static T_Page_Opr g_tMainPageOpr = {
	.c_pcName   = "mainpage",
	.Prepare    = Main_Page_Prepare,
	.Run		= Main_Page_Run,
	.Get_Input_Event = Main_Page_Get_Input_Event,
};

int Main_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tMainPageOpr);
}

