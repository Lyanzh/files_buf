#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ICON_X(w, t, i)	((Selected_Display()->tDevAttr.dwXres - w) / 2)
#define ICON_Y(h, t, i)	((Selected_Display()->tDevAttr.dwYres - h * t) / (t + 1) * (i + 1) + (h * i))

static T_IconInfo t_SettingPageIcon[] = 
{
	{"icon/select_fold.bmp", 0, 0, 0, 0},
	{"icon/interval.bmp", 0, 0, 0, 0},
};

static T_PicRegion tPicRegSrc;
static T_PicRegion tPicRegDst;

static void Setting_Page_Prepare(void)
{

}

static void Setting_Page_Run(void)
{
	int i;
	int iIconNum;

	iIconNum = sizeof(t_SettingPageIcon) / sizeof(T_IconInfo);
	for (i = 0; i < iIconNum; i++) {
		Get_Format_Opr("bmp")->Get_Pic_Region(t_SettingPageIcon[i].pcName, &tPicRegSrc);

		tPicRegDst.dwWidth = Selected_Display()->tDevAttr.dwXres / 10;
		tPicRegDst.dwHeight = tPicRegDst.dwWidth;
		t_SettingPageIcon[i].iTopLeftX = ICON_X(tPicRegDst.dwWidth, iIconNum, i);
		t_SettingPageIcon[i].iTopLeftY = ICON_Y(tPicRegDst.dwHeight, iIconNum, i);
		t_SettingPageIcon[i].iBottomRightX = t_SettingPageIcon[i].iTopLeftX + tPicRegDst.dwWidth;
		t_SettingPageIcon[i].iBottomRightY = t_SettingPageIcon[i].iTopLeftY + tPicRegDst.dwHeight;
		
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, 0);
		Fb_Lcd_Show_Pic(t_SettingPageIcon[i].iTopLeftX, t_SettingPageIcon[i].iTopLeftY, &tPicRegDst);
	}
}

static void Setting_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	Input_Get_Key(&tInputEvent);
	if (tInputEvent.cCode == 'd') {
		printf("\nselect folder.\n");
	} else if (tInputEvent.cCode == 't') {
		printf("\nset timer.\n");
	}
}

static T_Page_Opr g_tSettingPageOpr = {
	.c_pcName   = "settingpage",
	.Prepare    = Setting_Page_Prepare,
	.Run		= Setting_Page_Run,
	.Get_Input_Event = Setting_Page_Get_Input_Event,
};

int Setting_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tSettingPageOpr);
}
