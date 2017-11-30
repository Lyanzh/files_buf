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

static int g_iPicY;

static pthread_mutex_t g_tZoomMutex;
static pthread_cond_t g_tZoomCond;
static pthread_mutex_t g_tFlipMutex;
static pthread_cond_t g_tFlipCond;

static PT_FileList g_ptFileListPreShow;
static float g_fZoomFactorPre = 1;
static float g_fZoomFactorCur = 1;

PT_Page_Mem g_ptPageMemCur;
PT_Page_Mem g_ptPageMemLager;
PT_Page_Mem g_ptPageMemSmaller;

PT_Page_Mem g_ptPageMemNextPic;
PT_Page_Mem g_ptPageMemPrePic;

static void Pic_Prepare_Larger(void)
{
	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	
	memcpy(g_ptPageMemSmaller->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemCur->dwMemSize);
	memcpy(g_ptPageMemCur->pcMem, g_ptPageMemLager->pcMem, g_ptPageMemLager->dwMemSize);

	/* 准备下一次放大的数据 */
	Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
	Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur+0.1);
	Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemLager->pcMem);
	Do_Free(tPicRegSrc.pcData);
}

static void Pic_Prepare_Smaller(void)
{
	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	
	memcpy(g_ptPageMemLager->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemCur->dwMemSize);
	memcpy(g_ptPageMemCur->pcMem, g_ptPageMemSmaller->pcMem, g_ptPageMemSmaller->dwMemSize);

	/* 准备下一次缩小的数据 */
	Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
	Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur-0.1);
	Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemSmaller->pcMem);
	Do_Free(tPicRegSrc.pcData);
}

static void Pic_Prepare_Pre(void)
{
	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	
	memcpy(g_ptPageMemNextPic->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemCur->dwMemSize);
	memcpy(g_ptPageMemCur->pcMem, g_ptPageMemPrePic->pcMem, g_ptPageMemPrePic->dwMemSize);

	/* 准备上一张的数据 */
	Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
	Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur);
	Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemPrePic->pcMem);
	Do_Free(tPicRegSrc.pcData);

	/* 准备下一次缩小的数据 */
	Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
	Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur-0.1);
	Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemSmaller->pcMem);
	Do_Free(tPicRegSrc.pcData);
}

static void Pic_Prepare_Next(void)
{
	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	
	memcpy(g_ptPageMemPrePic->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemCur->dwMemSize);
	memcpy(g_ptPageMemCur->pcMem, g_ptPageMemNextPic->pcMem, g_ptPageMemNextPic->dwMemSize);

	/* 准备下一张的数据 */
	Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
	Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur);
	Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemNextPic->pcMem);
	Do_Free(tPicRegSrc.pcData);

	Pic_Prepare_Smaller();
	Pic_Prepare_Larger();


}

static void Browse_Page_Zoom_Thread(void *arg)
{
	while (1) {
		pthread_mutex_lock(&g_tZoomMutex);
		pthread_cond_wait(&g_tZoomCond, &g_tZoomMutex);
		
		if (!g_ptFileListCurShow) {
			continue;
		}

		printf("g_fZoomFactorCur = %f, g_fZoomFactorPre = %f\n", g_fZoomFactorCur, g_fZoomFactorPre);
		/* 显示这一次的，准备下一次的 */
		if (g_fZoomFactorCur > g_fZoomFactorPre) {/* 放大 */
			Lcd_Mem_Flush(g_ptPageMemLager);
			if (g_fZoomFactorCur <= 3) {
				Pic_Prepare_Larger();
			}
		} else if (g_fZoomFactorCur < g_fZoomFactorPre) {/* 缩小 */
			Lcd_Mem_Flush(g_ptPageMemSmaller);
			if (g_fZoomFactorCur > 0.1) {
				Pic_Prepare_Smaller();
			}
		}
		pthread_mutex_unlock(&g_tZoomMutex);
	}
	pthread_detach(pthread_self());
}

static void Browse_Page_Flip_Thread(void *arg)
{
	while (1) {
		pthread_mutex_lock(&g_tFlipMutex);
		pthread_cond_wait(&g_tFlipCond, &g_tFlipMutex);
		
		if (!g_ptFileListCurShow) {
			continue;
		}

		/* 显示这一次的，准备下一次的 */
		if (g_ptFileListCurShow == g_ptFileListPreShow->ptNext) {
			Lcd_Mem_Flush(g_ptPageMemNextPic);
			Pic_Prepare_Next();
		} else if (g_ptFileListCurShow == g_ptFileListPreShow->ptPre) {
			Lcd_Mem_Flush(g_ptPageMemPrePic);
			Pic_Prepare_Pre();
		}
		pthread_mutex_unlock(&g_tFlipMutex);
	}
	pthread_detach(pthread_self());
}

static int Browse_Page_Data(PT_Page_Mem ptPageMem)
{
	int i;
	int iIconNum;

	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;

	g_ptFileListPreShow = g_ptFileListCurShow;
	g_fZoomFactorPre = 1;
	g_fZoomFactorCur = 1;
	
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
		Do_Free(tPicRegSrc.pcData);
	}

	g_iPicY = tPicRegDst.dwHeight;
	if (g_ptFileListCurShow) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
		if (g_fZoomFactorCur == 0 || g_fZoomFactorCur == 1) {
			Lcd_Merge(0, g_iPicY, &tPicRegSrc, ptPageMem->pcMem);
		} else {
			Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur);
			Lcd_Merge(0, g_iPicY, &tPicRegDst, ptPageMem->pcMem);
		}
		Do_Free(tPicRegSrc.pcData);
	}
	
	ptPageMem->State = PAGE_MEM_PACKED;
	return 0;
}

static void Browse_Page_Pre_Thread(void)
{
#if 1
	PT_Page_Mem ptPageMem;
	/* 申请缓存 */
	ptPageMem = Page_Mem_Get(BROWSEPAGE_MAIN);
	if (ptPageMem) {
		/* 已经申请过，如果有数据则退出，否则写数据进缓存 */
		if (ptPageMem->State == PAGE_MEM_PACKED) {
			return;
		} else {
			/* 写数据进缓存 */
			Browse_Page_Data(ptPageMem);
		}
	} else {
		/* 还未申请或已经销毁，重新申请，并且写数据进去 */
		ptPageMem = Page_Mem_Alloc(BROWSEPAGE_MAIN);
		/* 写数据进缓存 */
		Browse_Page_Data(ptPageMem);
	}
#endif
	pthread_detach(pthread_self());
}

static void Browse_Page_PrepareSelf(void)
{
	pthread_t tThreadID;
	pthread_create(&tThreadID, NULL, (void*)Browse_Page_Pre_Thread, NULL);
}

static void Browse_Page_Run(void)
{
	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	
	pthread_t tShowTreadID;

	pthread_mutex_init(&g_tZoomMutex, NULL);
	pthread_cond_init(&g_tZoomCond, NULL);
	pthread_mutex_init(&g_tFlipMutex, NULL);
	pthread_cond_init(&g_tFlipCond, NULL);

	g_ptPageMemCur = Page_Mem_Get(BROWSEPAGE_MAIN);
	if (g_ptPageMemCur && g_ptPageMemCur->State == PAGE_MEM_PACKED) {
		Lcd_Mem_Flush(g_ptPageMemCur);
	} else {
		printf("Warning:Browsepage data has not been prepared\n");
	}

	/* 申请缓存 */
	g_ptPageMemLager = Page_Mem_Get(BROWSEPAGE_LARGER);
	if (!g_ptPageMemLager) {
		/* 还未申请或已经销毁，重新申请 */
		g_ptPageMemLager = Page_Mem_Alloc(BROWSEPAGE_LARGER);
		if (!g_ptPageMemLager) {
			printf("Error:can not get g_ptPageMemLager\n");
			return;
		}
	}
	g_ptPageMemSmaller = Page_Mem_Get(BROWSEPAGE_SMALLER);
	if (!g_ptPageMemSmaller) {
		/* 还未申请或已经销毁，重新申请 */
		g_ptPageMemSmaller = Page_Mem_Alloc(BROWSEPAGE_SMALLER);
		if (!g_ptPageMemSmaller) {
			printf("Error:can not get g_ptPageMemSmaller\n");
			return;
		}
	}
	g_ptPageMemNextPic = Page_Mem_Get(BROWSEPAGE_NEXT);
	if (!g_ptPageMemNextPic) {
		/* 还未申请或已经销毁，重新申请 */
		g_ptPageMemNextPic = Page_Mem_Alloc(BROWSEPAGE_NEXT);
		if (!g_ptPageMemNextPic) {
			printf("Error:can not get g_ptPageMemNextPic\n");
			return;
		}
	}
	g_ptPageMemPrePic = Page_Mem_Get(BROWSEPAGE_PRE);
	if (!g_ptPageMemPrePic) {
		/* 还未申请或已经销毁，重新申请 */
		g_ptPageMemPrePic = Page_Mem_Alloc(BROWSEPAGE_PRE);
		if (!g_ptPageMemPrePic) {
			printf("Error:can not get g_ptPageMemPrePic\n");
			return;
		}
	}

	memcpy(g_ptPageMemLager->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemLager->dwMemSize);
	memcpy(g_ptPageMemSmaller->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemSmaller->dwMemSize);
	memcpy(g_ptPageMemNextPic->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemNextPic->dwMemSize);
	memcpy(g_ptPageMemPrePic->pcMem, g_ptPageMemCur->pcMem, g_ptPageMemPrePic->dwMemSize);

	Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
	Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur+0.1);
	Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemLager->pcMem);

	Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur-0.1);
	Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemSmaller->pcMem);

	Do_Free(tPicRegSrc.pcData);

	if (g_ptFileListCurShow->ptPre) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->ptPre->pcName, &tPicRegSrc);
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur);
		Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemPrePic->pcMem);
		Do_Free(tPicRegSrc.pcData);
	}
	if (g_ptFileListCurShow->ptNext) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->ptNext->pcName, &tPicRegSrc);
		Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur);
		Lcd_Merge(0, g_iPicY, &tPicRegDst, g_ptPageMemNextPic->pcMem);
		Do_Free(tPicRegSrc.pcData);
	}
	
	pthread_create(&tShowTreadID, NULL, (void *)Browse_Page_Zoom_Thread, NULL);
	pthread_create(&tShowTreadID, NULL, (void *)Browse_Page_Flip_Thread, NULL);
}

static void Browse_Page_PrepareNext(void)
{
	Get_Page_Opr("mainpage")->PrepareSelf();
	Get_Page_Opr("autopage")->PrepareSelf();
}

static void Browse_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	while (1) {
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.cCode == 'l') {
			printf("\nlager.\n");
			pthread_mutex_lock(&g_tZoomMutex);
			g_fZoomFactorPre = g_fZoomFactorCur;
			g_fZoomFactorCur += 0.1;
			pthread_cond_signal(&g_tZoomCond);
			pthread_mutex_unlock(&g_tZoomMutex);
		} else if (tInputEvent.cCode == 's') {
			printf("\nsmaller.\n");
			if (g_fZoomFactorCur > 0.1) {
				pthread_mutex_lock(&g_tZoomMutex);
				g_fZoomFactorPre = g_fZoomFactorCur;
				g_fZoomFactorCur -= 0.1;
				pthread_cond_signal(&g_tZoomCond);
				pthread_mutex_unlock(&g_tZoomMutex);
			}
		} else if (tInputEvent.cCode == 'p') {
			if (g_ptFileListCurShow->ptPre) {
				pthread_mutex_lock(&g_tFlipMutex);
				g_ptFileListPreShow = g_ptFileListCurShow;
				g_ptFileListCurShow = g_ptFileListCurShow->ptPre;
				pthread_cond_signal(&g_tFlipCond);
				pthread_mutex_unlock(&g_tFlipMutex);
			}
		} else if (tInputEvent.cCode == 'n') {
			if (g_ptFileListCurShow->ptNext) {
				pthread_mutex_lock(&g_tFlipMutex);
				g_ptFileListPreShow = g_ptFileListCurShow;
				g_ptFileListCurShow = g_ptFileListCurShow->ptNext;
				pthread_cond_signal(&g_tFlipCond);
				pthread_mutex_unlock(&g_tFlipMutex);
			}
		} else if (tInputEvent.cCode == 'q') {
			Page_Change("mainpage");
		} else if(tInputEvent.cCode == 'a') {
			Page_Change("autopage");
		}
	}
}

static void Browse_Page_Exit(void)
{
	printf("Browse_Page_Exit\n");
	Page_Grop_Mem_List_Del(BROWSEPAGE_GROUP);
}

static T_Page_Opr g_tBrowsePageOpr = {
	.c_pcName        = "browsepage",
	.PrepareSelf     = Browse_Page_PrepareSelf,
	.PrepareNext     = Browse_Page_PrepareNext,
	.Run		     = Browse_Page_Run,
	.Get_Input_Event = Browse_Page_Get_Input_Event,
	.Exit            = Browse_Page_Exit,
};

int Browse_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tBrowsePageOpr);
}

