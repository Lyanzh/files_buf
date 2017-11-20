#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ZOOMOUT_X(w)				(0)
#define ZOOMOUT_Y(h)				(0)

#define ZOOMIN_X(w)					(1 * w)
#define ZOOMIN_Y(h)					(0)

#define CONTINUE_MODE_SMALL_X(w)	(2 * w)
#define CONTINUE_MODE_SMALL_Y(h)	(0)

#define PRE_PIC_X(w)				(3 * w)
#define PRE_PIC_Y(h)				(0)

#define NEXT_PIC_X(w)				(4 * w)
#define NEXT_PIC_Y(h)				(0)

T_IconInfo t_BrowsePageIcon[] = 
{
	{"icon/zoomout.bmp", 0, 0},
	{"icon/zoomin.bmp", 0, 0},
	{"icon/continue_mode_small.bmp", 0, 0},
	{"icon/pre_pic.bmp", 0, 0},
	{"icon/next_pic.bmp", 0, 0},
};

T_PicRegion tPicRegSrc;
T_PicRegion tPicRegDst;

int g_iPicY;

static pthread_mutex_t g_tShowMutex;
static pthread_cond_t g_tShowCond;

PT_FileList g_ptFileListCurShow;
float g_fZoomFactor;

static void Browse_Page_Prepare(void)
{
	char basePath[100];

	//get the current absoulte path
	memset(basePath, '\0', sizeof(basePath));
	getcwd(basePath, 99);
	printf("the current dir is : %s\n", basePath);

	//get the file list
	Read_File_List(basePath);

	Show_File_List();

	g_ptFileListCurShow = g_ptFileListHead;
	g_fZoomFactor = 1;
}

static void *Browse_Page_Thread(void *arg)
{
	while (1) {
		pthread_mutex_lock(&g_tShowMutex);
		pthread_cond_wait(&g_tShowCond, &g_tShowMutex);
		if (g_ptFileListCurShow) {
			Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
			if (g_fZoomFactor == 0 || g_fZoomFactor == 1) {
				Fb_Lcd_Show_Pic(0, g_iPicY, &tPicRegSrc);
			} else {
				Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactor);
				Fb_Lcd_Show_Pic(0, g_iPicY, &tPicRegDst);
			}
			//free(tBrowseModeSrc.pcData);
			//free(tBrowseModeDst.pcData);
		}
		pthread_mutex_unlock(&g_tShowMutex);
	}
	return NULL;
}

static void Browse_Page_Run(void)
{
	int i;
	int iIconNum;
	pthread_t tShowTreadID;

	pthread_mutex_init(&g_tShowMutex, NULL);
	pthread_cond_init(&g_tShowCond, NULL);

	iIconNum = sizeof(t_BrowsePageIcon) / sizeof(T_IconInfo);
	for (i = 0; i < iIconNum; i++) {
		Get_Format_Opr("bmp")->Get_Pic_Region(t_BrowsePageIcon[i].pcName, &tPicRegSrc);
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, 0.5);
		Fb_Lcd_Show_Pic(tPicRegDst.dwWidth * i, t_BrowsePageIcon[i].iY, &tPicRegDst);
	}

	g_iPicY = tPicRegDst.dwHeight;

	if (g_ptFileListCurShow) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
		if (g_fZoomFactor == 0 || g_fZoomFactor == 1) {
			Fb_Lcd_Show_Pic(0, g_iPicY, &tPicRegSrc);
		} else {
			Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactor);
			Fb_Lcd_Show_Pic(0, g_iPicY, &tPicRegDst);
		}
		
		//free(tBrowseModeSrc.pcData);
		//free(tBrowseModeDst.pcData);
	}
	
	pthread_create(&tShowTreadID, NULL, Browse_Page_Thread, NULL);
}

static void Browse_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
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
	}
}

static T_Page_Opr g_tBrowsePageOpr = {
	.c_pcName   = "browsepage",
	.Prepare    = Browse_Page_Prepare,
	.Run		= Browse_Page_Run,
	.Get_Input_Event = Browse_Page_Get_Input_Event,
};

int Browse_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tBrowsePageOpr);
}

