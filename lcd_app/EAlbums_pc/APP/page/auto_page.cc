#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>

typedef void (*sighandler_t)(int);

PT_Page_Mem g_ptPageMemCurPic;
PT_Page_Mem g_ptPageMemNextPic;

//static T_PicRegion tPicRegSrc;
//static T_PicRegion tPicRegDst;

static int g_iPicY;

static pthread_mutex_t g_tShowMutex;
static pthread_cond_t g_tShowCond;

static struct itimerval g_tOldTv;

static int Auto_Page_Data(PT_Page_Mem ptPageMem)
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

	g_iPicY = 0;

	if (g_ptFileListCurShow) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
		Lcd_Merge(0, g_iPicY, &tPicRegSrc, ptPageMem->pcMem);
	}
	ptPageMem->State = PAGE_MEM_PACKED;
	return 0;
}

static void Auto_Page_Pre_Thread(void)
{
#if 1
	PT_Page_Mem ptPageMem;
	/* 申请缓存 */
	ptPageMem = Page_Mem_Get(AUTOPAGE_MAIN);
	if (ptPageMem) {
		/* 已经申请过，如果有数据则退出，否则写数据进缓存 */
		if (ptPageMem->State == PAGE_MEM_PACKED) {
			return;
		} else {
			/* 写数据进缓存 */
			Auto_Page_Data(ptPageMem);
		}
	} else {
		/* 还未申请或已经销毁，重新申请，并且写数据进去 */
		ptPageMem = Page_Mem_Alloc(AUTOPAGE_MAIN);
		/* 写数据进缓存 */
		Auto_Page_Data(ptPageMem);
	}
#endif
	pthread_detach(pthread_self());
}

static void Auto_Page_Prepare(void)
{
	pthread_t tThreadID;
	pthread_create(&tThreadID, NULL, (void*)Auto_Page_Pre_Thread, NULL);
}
  
static void Set_Timer(void)
{
	struct itimerval tTv;
	tTv.it_interval.tv_sec = 5;	/* 定时器间隔为1s */
	tTv.it_interval.tv_usec = 0;
	tTv.it_value.tv_sec = 5;	/* 1秒后启用定时器 */
	tTv.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tTv, &g_tOldTv);
}

static void signal_handler(void)
{
	if (g_ptFileListCurShow->ptNext) {
		pthread_mutex_lock(&g_tShowMutex);
		g_ptFileListCurShow = g_ptFileListCurShow->ptNext;
		pthread_cond_signal(&g_tShowCond);
		pthread_mutex_unlock(&g_tShowMutex);
	}
}

static void Pic_Prepare_Next(void)
{
	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	
	memcpy(g_ptPageMemCurPic->pcMem, g_ptPageMemNextPic->pcMem, g_ptPageMemNextPic->dwMemSize);

	/* 准备下一张的数据 */
	Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
	//Pic_Zoom(&tPicRegDst, &tPicRegSrc, g_fZoomFactorCur);
	Lcd_Merge(0, g_iPicY, &tPicRegSrc, g_ptPageMemNextPic->pcMem);
	Do_Free(tPicRegSrc.pcData);
}

static void *Auto_Page_Thread(void *arg)
{
	while (1) {
		pthread_mutex_lock(&g_tShowMutex);
		pthread_cond_wait(&g_tShowCond, &g_tShowMutex);
		Lcd_Mem_Flush(g_ptPageMemNextPic);
		Pic_Prepare_Next();
		pthread_mutex_unlock(&g_tShowMutex);
	}
	return NULL;
}

static void Auto_Page_Run(void)
{
	T_PicRegion tPicRegSrc;
	T_PicRegion tPicRegDst;
	pthread_t tShowTreadID;

	pthread_mutex_init(&g_tShowMutex, NULL);
	pthread_cond_init(&g_tShowCond, NULL);

	/* 获取当前一张缓存 */
	g_ptPageMemCurPic = Page_Mem_Get(AUTOPAGE_MAIN);
	if (g_ptPageMemCurPic && g_ptPageMemCurPic->State == PAGE_MEM_PACKED) {
		Lcd_Mem_Flush(g_ptPageMemCurPic);
	} else {
		printf("Warning:Autopage data has not been prepared\n");
	}

	signal(SIGALRM, (sighandler_t)signal_handler);
	Set_Timer();
	pthread_create(&tShowTreadID, NULL, Auto_Page_Thread, NULL);

	/* 申请下一张缓存 */
	g_ptPageMemNextPic = Page_Mem_Get(AUTOPAGE_NEXT);
	if (!g_ptPageMemNextPic) {
		/* 还未申请或已经销毁，重新申请 */
		g_ptPageMemNextPic = Page_Mem_Alloc(AUTOPAGE_NEXT);
		if (!g_ptPageMemNextPic) {
			printf("Error:can not get g_ptPageMemNextPic\n");
			return;
		}
	}
	if (g_ptFileListCurShow->ptNext) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->ptNext->pcName, &tPicRegSrc);
		Lcd_Merge(0, g_iPicY, &tPicRegSrc, g_ptPageMemNextPic->pcMem);
		Do_Free(tPicRegSrc.pcData);
	}
}

static void Auto_Page_PrepareNext(void)
{
	Get_Page_Opr("browsepage")->PrepareSelf();
}

static void Auto_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;

	while (1) {
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.cCode == 'b') {
			printf("\nto browse page.\n");
			Page_Change("browsepage");
		}
	}
}

static void Auto_Page_Exit(void)
{
	Page_Grop_Mem_List_Del(AUTOPAGE_GROUP);
}

static T_Page_Opr g_tAutoPageOpr = {
	.c_pcName        = "autopage",
	.PrepareSelf     = Auto_Page_Prepare,
	.PrepareNext     = Auto_Page_PrepareNext,
	.Run		     = Auto_Page_Run,
	.Get_Input_Event = Auto_Page_Get_Input_Event,
	.Exit            = Auto_Page_Exit,
};

int Auto_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tAutoPageOpr);
}

