#include "input_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "memwatch.h"

static PT_Input_Opr g_ptInputOprHead;
T_Input_Event g_tInputEvent;

pthread_mutex_t g_tMutex; /* 互斥体lock 用于对缓冲区的互斥操作 */
pthread_cond_t g_tInputEvent; /* 缓冲区非空的条件变量 */

void *Input_Get_Key(void *arg);

int Input_Opr_Regisiter(PT_Input_Opr ptInputOpr)
{
	PT_Input_Opr ptInputOprTmp;
	
	if (!g_ptInputOprHead) {
		g_ptInputOprHead = ptInputOpr;
	} else {
		ptInputOprTmp = g_ptInputOprHead;
		while (ptInputOprTmp->ptNext) {
			ptInputOprTmp = ptInputOprTmp->ptNext;
		}
		ptInputOprTmp->ptNext = ptInputOpr;
	}
	ptInputOpr->ptNext = NULL;

	return 0;
}

void Show_Input_Opr(void)
{
	int i = 0;
	PT_Input_Opr ptInputOprTmp = g_ptInputOprHead;
	while (ptInputOprTmp) {
		printf("%d: %s\n", i++, ptInputOprTmp->c_pcName);
		ptInputOprTmp = ptInputOprTmp->ptNext;
	}
}

PT_Input_Opr Get_Input_Opr(char *pcName)
{
	PT_Input_Opr ptInputOprTmp = g_ptInputOprHead;
	while (ptInputOprTmp) {
		if (strcmp(ptInputOprTmp->c_pcName, pcName) == 0) {
			printf("get input %s.\n", pcName);
			return ptInputOprTmp;
		} else {
			ptInputOprTmp = ptInputOprTmp->ptNext;
		}
	}
	printf("Error:can't get input %s.\n", pcName);
	return NULL;
}

int Input_Opr_Init(void)
{
	int iError;
	iError = Stdin_Input_Init();
	if (iError) {
		printf("Error:Stdin init fail.\n");
		return -1;
	}

	iError = Touchscreen_Input_Init();
	if (iError) {
		printf("Error:Stdin init fail.\n");
		return -1;
	}
	return 0;
}

int All_Input_Device_Init(void)
{
	int iError = -1;
	int iRet;
	PT_Input_Opr ptInputOprTmp;

	pthread_t thread;

    pthread_mutex_init(&g_Mutex, NULL);
    pthread_cond_init(&notempty, NULL);

	if (!g_ptInputOprHead) {
		iError = -1;
	} else {
		ptInputOprTmp = g_ptInputOprHead;
		while (ptInputOprTmp) {
			iRet = ptInputOprTmp->Input_Init();
			if (0 == iRet) {
				pthread_create(&thread, NULL, thread_function, (void *)ptInputOprTmp);
				iError = 0;
			}
			ptInputOprTmp = ptInputOprTmp->ptNext;
		}
	}

	printf("All_Input_Device_Init over.\n");

	return iError;
}

static int Input_Get_InputEvent(PT_Input_Event ptInputEvent, PT_Input_Data ptInputData)
{
	if (!ptInputEvent || !ptInputData)
		return -1;

	ptInputEvent->iType = ptInputData->iType;
	gettimeofday(&ptInputEvent->tTime, NULL);
	if (ptInputEvent->iType == INPUT_TYPE_STDIN) {
		if (ptInputData->cCode == 'u') {
			ptInputEvent->iVal = INPUT_VALUE_UP;
		} else if (ptInputData->cCode == 'n') {
			ptInputEvent->iVal = INPUT_VALUE_DOWN;
		} else if (ptInputData->cCode == 'q') {
			ptInputEvent->iVal = INPUT_VALUE_EXIT;
		} else {
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		}
	} else if (ptInputEvent->iType == INPUT_TYPE_TOUCHSCREEN) {
		//...
	}

	return 0;
}

#if 0
int Input_Get_Key(PT_Input_Event ptInputEvent)
{
   
	PT_Input_Opr ptInputOprTmp;
	T_Input_Data tInputData;
	
	ptInputOprTmp = g_ptInputOprHead;


	if (ptInputOprTmp->Input_Get_Data(&tInputData)) {
		if (0 == Input_Get_InputEvent(ptInputEvent, &tInputData))
			return 1;
		else
			return -1;
	} else {
		return 0;
	}

	return 0;
}
#endif

void *Input_Get_Key(void *arg)
{
	PT_Input_Opr ptInputOprTmp;
	T_Input_Data tInputData;
	
	ptInputOprTmp = (PT_Input_Opr)arg;

	while (1) {
		if (ptInputOprTmp->Input_Get_Data(&tInputData)) {
			pthread_mutex_lock(&g_tMutex);
			if (Input_Get_InputEvent(&g_tInputEvent, &tInputData)) {
				printf("Error:get input event error.\n");
			}
			pthread_cond_signal(&g_tInputEvent);
			pthread_mutex_unlock(&g_tMutex);
		}
	}
}

