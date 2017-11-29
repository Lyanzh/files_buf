#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ICON_X(w, t, i)	((Selected_Display()->tDevAttr.dwXres - w) / 2)
#define ICON_Y(h, t, i)	((Selected_Display()->tDevAttr.dwYres - h * t) / (t + 1) * (i + 1) + (h * i))

static T_IconInfo t_MainPageIcon[] = 
{
	{"icon/browse_mode.bmp", 0, 0, 0, 0},
	{"icon/continue_mode.bmp", 0, 0, 0, 0},
	{"icon/setting.bmp", 0, 0, 0, 0},
};

static int Main_Page_Data(PT_Page_Mem ptPageMem)
{
	int i;
	int iIconNum;

	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	
	if (!ptPageMem) {
		printf("Error:Mainpage memery invalid\n");
		return -1;
	}

	if (ptPageMem->State == PAGE_MEM_PACKED) {
		printf("Warning:Mainpage memery has data\n");
		return 0;
	}

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
		Lcd_Merge(t_MainPageIcon[i].iTopLeftX, t_MainPageIcon[i].iTopLeftY,
				&tPicRegDst, ptPageMem->pcMem);

		printf("tPicRegDst->pcData = 0x%x\n", tPicRegDst.pcData);
		printf("tPicRegSrc->pcData = 0x%x\n", tPicRegSrc.pcData);
		//Do_Free(tPicRegSrc.pcData);
		Do_Free(tPicRegSrc.pcData);
	}
	ptPageMem->State = PAGE_MEM_PACKED;
	
	return 0;
}

static void Main_Page_Pre_Thread(void)
{
	PT_Page_Mem ptPageMem;
	/* 申请缓存 */
	ptPageMem = Page_Mem_Get(MAINPAGE_MAIN);
	if (ptPageMem) {
		/* 已经申请过，如果有数据则退出，否则写数据进缓存 */
		if (ptPageMem->State == PAGE_MEM_PACKED) {
			return;
		} else {
			/* 写数据进缓存 */
			Main_Page_Data(ptPageMem);
		}
	} else {
		/* 还未申请或已经销毁，重新申请，并且写数据进去 */
		ptPageMem = Page_Mem_Alloc(MAINPAGE_MAIN);
		/* 写数据进缓存 */
		Main_Page_Data(ptPageMem);
	}
	pthread_detach(pthread_self());
}

static void Main_Page_PrepareSelf(void)
{
	pthread_t tThreadID;
	pthread_create(&tThreadID, NULL, (void*)Main_Page_Pre_Thread, NULL);
}

static void Main_Page_Run(void)
{
	PT_Page_Mem ptPageMem;

	ptPageMem = Page_Mem_Get(MAINPAGE_MAIN);
	if (ptPageMem && ptPageMem->State == PAGE_MEM_PACKED) {
		Lcd_Mem_Flush(ptPageMem);
	} else {
		printf("Warning:Mainpage data has not been prepared\n");
	}
}

static void Main_Page_PrepareNext(void)
{
	Get_Page_Opr("browsepage")->PrepareSelf();
	Get_Page_Opr("settingpage")->PrepareSelf();
}

static void Main_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	while (1) {
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.cCode == 'b') {
			printf("\nshow browse page.\n");
			Page_Change("browsepage");
		} else if (tInputEvent.cCode == 'c') {
			printf("\nshow continue mode page.\n");
		} else if (tInputEvent.cCode == 's') {
			printf("\nshow setting page.\n");
			Page_Change("settingpage");
		}
	}
}

static void Main_Page_Exit(void)
{
	Page_Grop_Mem_List_Del(MAINPAGE_GROUP);
}

static T_Page_Opr g_tMainPageOpr = {
	.c_pcName        = "mainpage",
	.PrepareSelf     = Main_Page_PrepareSelf,
	.PrepareNext     = Main_Page_PrepareNext,
	.Run		     = Main_Page_Run,
	.Get_Input_Event = Main_Page_Get_Input_Event,
	.Exit            = Main_Page_Exit,
};

int Main_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tMainPageOpr);
}

