#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>

typedef void (*sighandler_t)(int);

static T_PicRegion tPicRegSrc;
static T_PicRegion tPicRegDst;

static int g_iPicY;

static pthread_mutex_t g_tShowMutex;
static pthread_cond_t g_tShowCond;

static PT_FileList g_ptFileListCurShow;

static struct itimerval g_tOldTv;

static void Auto_Page_Prepare(void)
{
	char acBasePath[100];

	//get the current absoulte path
	memset(acBasePath, '\0', sizeof(acBasePath));
	getcwd(acBasePath, 99);
	printf("the current dir is : %s\n", acBasePath);

	//get the file list
	Read_File_List(acBasePath);

	Show_File_List();

	g_ptFileListCurShow = g_ptFileListHead;
}
  
void Set_Timer(void)
{
	struct itimerval tTv;
	tTv.it_interval.tv_sec = 5;	/* 定时器间隔为1s */
	tTv.it_interval.tv_usec = 0;
	tTv.it_value.tv_sec = 5;	/* 1秒后启用定时器 */
	tTv.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tTv, &g_tOldTv);
}
  
void signal_handler(void)
{  
	pthread_mutex_lock(&g_tShowMutex);
	g_ptFileListCurShow = g_ptFileListCurShow->ptPre;
	pthread_cond_signal(&g_tShowCond);
	pthread_mutex_unlock(&g_tShowMutex);
}

static void *Auto_Page_Thread(void *arg)
{
	while (1) {
		pthread_mutex_lock(&g_tShowMutex);
		pthread_cond_wait(&g_tShowCond, &g_tShowMutex);
		if (g_ptFileListCurShow) {
			Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
			Lcd_Show_Pic(0, g_iPicY, &tPicRegSrc);
		}
		pthread_mutex_unlock(&g_tShowMutex);
	}
	return NULL;
}

static void Auto_Page_Run(void)
{
	pthread_t tShowTreadID;

	pthread_mutex_init(&g_tShowMutex, NULL);
	pthread_cond_init(&g_tShowCond, NULL);

	g_iPicY = tPicRegDst.dwHeight;

	if (g_ptFileListCurShow) {
		Get_Format_Opr("jpeg")->Get_Pic_Region(g_ptFileListCurShow->pcName, &tPicRegSrc);
		Lcd_Show_Pic(0, 0, &tPicRegSrc);
		printf("Lcd_Show_Pic\n");
	}

	signal(SIGALRM, (sighandler_t)signal_handler);

	Set_Timer();
	
	pthread_create(&tShowTreadID, NULL, Auto_Page_Thread, NULL);
}

static void Auto_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;

	while (1) {
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.cCode == 'q') {
			printf("\nto browse page.\n");
		}
	}
}

static T_Page_Opr g_tAutoPageOpr = {
	.c_pcName   = "autopage",
	.Prepare    = Auto_Page_Prepare,
	.Run		= Auto_Page_Run,
	.Get_Input_Event = Auto_Page_Get_Input_Event,
};

int Auto_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tAutoPageOpr);
}

