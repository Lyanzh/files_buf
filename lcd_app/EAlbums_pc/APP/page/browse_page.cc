#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ICON_X(w, t, i)	(w * i)
#define ICON_Y(h, t, i)	(0)

static T_IconInfo t_BrowsePageIcon[] = 
{
	{"icon/return.bmp", 0, 0, 0, 0},
	{"icon/zoomout.bmp", 0, 0, 0, 0},
	{"icon/zoomin.bmp", 0, 0, 0, 0},
	{"icon/continue_mode_small.bmp", 0, 0, 0, 0},
	{"icon/pre_pic.bmp", 0, 0, 0, 0},
	{"icon/next_pic.bmp", 0, 0, 0, 0},
};

static T_PicRegion tPicRegSrc;
static T_PicRegion tPicRegDst;

static int g_iPicY;

static pthread_mutex_t g_tShowMutex;
static pthread_cond_t g_tShowCond;

static PT_FileList g_ptFileListCurShow;
static float g_fZoomFactor;

static void *Browse_Page_Thread(void *arg)
{
	while (1) {
		pthread_mutex_lock(&g_tShowMutex);
		pthread_cond_wait(&g_tShowCond, &g_tShowMutex);
		if (g_ptFileListCurShow) {
			Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
			if (g_fZoomFactor == 0 || g_fZoomFactor == 1) {
				Lcd_Show_Pic(0, g_iPicY, &tPicRegSrc);
			} else {
				Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactor);
				Lcd_Show_Pic(0, g_iPicY, &tPicRegDst);
			}
		}
		pthread_mutex_unlock(&g_tShowMutex);
	}
	return NULL;
}

static int Browse_Page_Data(PT_Page_Mem ptPageMem)
{
	int i;
	int iIconNum;
	char acBasePath[100];

	//get the current absoulte path
	memset(acBasePath, '\0', sizeof(acBasePath));
	getcwd(acBasePath, 99);
	printf("the current dir is : %s\n", acBasePath);

	//get the file list
	Read_File_List(acBasePath);

	Show_File_List();

	g_ptFileListCurShow = g_ptFileListHead;
	g_fZoomFactor = 1;
	
	if (!ptPageMem) {
		printf("Error:Mainpage memery invalid\n");
		return -1;
	}

	if (ptPageMem->State == PAGE_MEM_PACKED) {
		printf("Warning:Mainpage memery has data\n");
		return 0;
	}

	iIconNum = sizeof(t_BrowsePageIcon) / sizeof(T_IconInfo);
	for (i = 0; i < iIconNum; i++) {
		Get_Format_Opr("bmp")->Get_Pic_Region(t_BrowsePageIcon[i].pcName, &tPicRegSrc);
		tPicRegDst.dwWidth = Selected_Display()->tDevAttr.dwXres / 10;
		tPicRegDst.dwHeight = tPicRegDst.dwWidth;
		t_BrowsePageIcon[i].iTopLeftX = ICON_X(tPicRegDst.dwWidth, iIconNum, i);
		t_BrowsePageIcon[i].iTopLeftY = ICON_Y(tPicRegDst.dwHeight, iIconNum, i);
		t_BrowsePageIcon[i].iBottomRightX = t_BrowsePageIcon[i].iTopLeftX + tPicRegDst.dwWidth;
		t_BrowsePageIcon[i].iBottomRightY = t_BrowsePageIcon[i].iTopLeftY + tPicRegDst.dwHeight;
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, 0);
		Lcd_Merge(t_BrowsePageIcon[i].iTopLeftX, t_BrowsePageIcon[i].iTopLeftY, &tPicRegDst, ptPageMem->pcMem);
	}

	g_iPicY = tPicRegDst.dwHeight;
	if (g_ptFileListCurShow) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
		if (g_fZoomFactor == 0 || g_fZoomFactor == 1) {
			Lcd_Merge(0, g_iPicY, &tPicRegSrc, ptPageMem->pcMem);
		} else {
			Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactor);
			Lcd_Merge(0, g_iPicY, &tPicRegDst, ptPageMem->pcMem);
		}
	}
	
	ptPageMem->State = PAGE_MEM_PACKED;
	return 0;
}

static void Browse_Page_Pre_Thread(void)
{
	PT_Page_Mem ptPageMem;
	/* ���뻺�� */
	ptPageMem = Page_Mem_Get(BROWSEPAGE_MAIN);
	if (ptPageMem) {
		/* �Ѿ��������������������˳�������д���ݽ����� */
		if (ptPageMem->State == PAGE_MEM_PACKED) {
			return;
		} else {
			/* д���ݽ����� */
			Browse_Page_Data(ptPageMem);
		}
	} else {
		/* ��δ������Ѿ����٣��������룬����д���ݽ�ȥ */
		ptPageMem = Page_Mem_Alloc(BROWSEPAGE_MAIN);
		/* д���ݽ����� */
		Browse_Page_Data(ptPageMem);
	}
	pthread_detach(pthread_self());
}

static void Browse_Page_PrepareSelf(void)
{
	pthread_t tThreadID;
	pthread_create(&tThreadID, NULL, (void*)Browse_Page_Pre_Thread, NULL);
}

static void Browse_Page_Run(void)
{
	PT_Page_Mem ptPageMem;
	pthread_t tShowTreadID;

	pthread_mutex_init(&g_tShowMutex, NULL);
	pthread_cond_init(&g_tShowCond, NULL);

	ptPageMem = Page_Mem_Get(BROWSEPAGE_MAIN);
	if (ptPageMem && ptPageMem->State == PAGE_MEM_PACKED) {
		Lcd_Mem_Flush(ptPageMem);
		ptPageMem->State = PAGE_MEM_BUSY;
	} else {
		printf("Warning:Browsepage data has not been prepared\n");
	}
	
	pthread_create(&tShowTreadID, NULL, Browse_Page_Thread, NULL);
}

static void Browse_Page_PrepareNext(void)
{
	//Get_Page_Opr("settingpage")->PrepareSelf();
}

static void Browse_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	while (1) {
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.cCode == 'l') {
			printf("\nlager.\n");
			pthread_mutex_lock(&g_tShowMutex);
			g_fZoomFactor += 0.1;
			pthread_cond_signal(&g_tShowCond);
			pthread_mutex_unlock(&g_tShowMutex);
		} else if (tInputEvent.cCode == 's') {
			printf("\nsmaller.\n");
			if (g_fZoomFactor > 0.1) {
				pthread_mutex_lock(&g_tShowMutex);
				g_fZoomFactor -= 0.1;
				pthread_cond_signal(&g_tShowCond);
				pthread_mutex_unlock(&g_tShowMutex);
			}
		} else if (tInputEvent.cCode == 'p') {
			pthread_mutex_lock(&g_tShowMutex);
			g_ptFileListCurShow = g_ptFileListCurShow->ptPre;
			pthread_cond_signal(&g_tShowCond);
			pthread_mutex_unlock(&g_tShowMutex);
		} else if (tInputEvent.cCode == 'n') {
			pthread_mutex_lock(&g_tShowMutex);
			g_ptFileListCurShow = g_ptFileListCurShow->ptNext;
			pthread_cond_signal(&g_tShowCond);
			pthread_mutex_unlock(&g_tShowMutex);
		} else if (tInputEvent.cCode == 'q') {
			Page_Change("mainpage");
		}
	}
}

static T_Page_Opr g_tBrowsePageOpr = {
	.c_pcName        = "browsepage",
	.PrepareSelf     = Browse_Page_PrepareSelf,
	.PrepareNext     = Browse_Page_PrepareNext,
	.Run		     = Browse_Page_Run,
	.Get_Input_Event = Browse_Page_Get_Input_Event,
};

int Browse_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tBrowsePageOpr);
}

