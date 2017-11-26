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

static int Setting_Page_Date(PT_Page_Mem ptPageMem)
{
	int i;
	int iIconNum;
	if (!ptPageMem) {
		printf("Error:Settingpage memery invalid\n");
		return -1;
	}

	if (ptPageMem->State == PAGE_MEM_PACKED) {
		printf("Warning:Settingpage memery has data\n");
		return 0;
	}

	iIconNum = sizeof(t_SettingPageIcon) / sizeof(T_IconInfo);
	for (i = 0; i < iIconNum; i++) {
		Get_Format_Opr("bmp")->Get_Pic_Region(t_SettingPageIcon[i].pcName, &tPicRegSrc);

		tPicRegDst.dwWidth = Selected_Display()->tDevAttr.dwXres / 5;
		tPicRegDst.dwHeight = tPicRegDst.dwWidth / 2;
		t_SettingPageIcon[i].iTopLeftX = ICON_X(tPicRegDst.dwWidth, iIconNum, i);
		t_SettingPageIcon[i].iTopLeftY = ICON_Y(tPicRegDst.dwHeight, iIconNum, i);
		t_SettingPageIcon[i].iBottomRightX = t_SettingPageIcon[i].iTopLeftX + tPicRegDst.dwWidth;
		t_SettingPageIcon[i].iBottomRightY = t_SettingPageIcon[i].iTopLeftY + tPicRegDst.dwHeight;
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, 0);
		//Lcd_Show_Pic(t_SettingPageIcon[i].iTopLeftX, t_SettingPageIcon[i].iTopLeftY, &tPicRegDst);
		Lcd_Merge(t_SettingPageIcon[i].iTopLeftX, t_SettingPageIcon[i].iTopLeftY,
				&tPicRegDst, ptPageMem->pcMem);
	}
	
	ptPageMem->State = PAGE_MEM_PACKED;
	return 0;
}

static void Setting_Page_Pre_Thread(void)
{
	PT_Page_Mem ptPageMem;
	/* 申请缓存 */
	ptPageMem = Page_Mem_Get(SETTINGPAGE_MAIN);
	if (ptPageMem) {
		/* 已经申请过，如果有数据则退出，否则写数据进缓存 */
		if (ptPageMem->State == PAGE_MEM_PACKED) {
			return;
		} else {
			/* 写数据进缓存 */
			Setting_Page_Date(ptPageMem);
		}
	} else {
		/* 还未申请或已经销毁，重新申请，并且写数据进去 */
		ptPageMem = Page_Mem_Alloc(SETTINGPAGE_MAIN);
		/* 写数据进缓存 */
		Setting_Page_Date(ptPageMem);
	}
	pthread_detach(pthread_self());
}

static void Setting_Page_PrepareSelf(void)
{
	pthread_t tThreadID;
	pthread_create(&tThreadID, NULL, (void*)Setting_Page_Pre_Thread, NULL);
}

static void Setting_Page_Run(void)
{
	PT_Page_Mem ptPageMem;

	ptPageMem = Page_Mem_Get(SETTINGPAGE_MAIN);
	if (ptPageMem && ptPageMem->State == PAGE_MEM_PACKED) {
		Lcd_Mem_Flush(ptPageMem);
		ptPageMem->State = PAGE_MEM_BUSY;
	} else {
		printf("Warning:Settingpage data has not been prepared\n");
	}
}

static void Setting_Page_PrepareNext(void)
{
	//Get_Page_Opr("settingpage")->PrepareSelf();
}

static void Setting_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	while (1) {
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.cCode == 'd') {
			printf("\nselect folder.\n");
		} else if (tInputEvent.cCode == 't') {
			printf("\nset timer.\n");
		}
	}
}

static T_Page_Opr g_tSettingPageOpr = {
	.c_pcName        = "settingpage",
	.PrepareSelf     = Setting_Page_PrepareSelf,
	.PrepareNext     = Setting_Page_PrepareNext,
	.Run		     = Setting_Page_Run,
	.Get_Input_Event = Setting_Page_Get_Input_Event,
};

int Setting_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tSettingPageOpr);
}
