#include "input_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "memwatch.h"

static PT_Input_Opr g_ptInputOprHead;
T_Input_Event g_tInputEvent;

pthread_mutex_t g_tMutex; /* ������lock ���ڶԻ������Ļ������ */
pthread_cond_t g_tInputCond; /* �������ǿյ��������� */

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
		printf("Error:Touchscreen init fail.\n");
		return -1;
	}
	return 0;
}

static int Input_Get_InputEvent(PT_Input_Event ptInputEvent, PT_Input_Data ptInputData)
{
	static unsigned int dwPressure = 0;

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
		printf("pressure = %d\n", ptInputData->dwPressure);
		if (ptInputData->dwPressure == 1 && ptInputData->dwPressure != dwPressure) {
			ptInputEvent->iVal = INPUT_VALUE_DOWN;
			dwPressure = ptInputData->dwPressure;
		} else {
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		}
	}

	return 0;
}

static void *Input_Thread_Function(void *arg)
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
			pthread_cond_signal(&g_tInputCond);
			pthread_mutex_unlock(&g_tMutex);
		}
	}
}

int All_Input_Device_Init(void)
{
	int iError = -1;
	int iRet;
	PT_Input_Opr ptInputOprTmp;

    pthread_mutex_init(&g_tMutex, NULL);
    pthread_cond_init(&g_tInputCond, NULL);

	if (!g_ptInputOprHead) {
		iError = -1;
	} else {
		ptInputOprTmp = g_ptInputOprHead;
		while (ptInputOprTmp) {
			iRet = ptInputOprTmp->Input_Init();
			if (0 == iRet) {
				printf("pthread_create: %s\n", ptInputOprTmp->c_pcName);
				pthread_create(&ptInputOprTmp->tTreadID, NULL, Input_Thread_Function, (void *)ptInputOprTmp);
				iError = 0;
			}
			ptInputOprTmp = ptInputOprTmp->ptNext;
		}
	}

	printf("All_Input_Device_Init over.\n");

	return iError;
}

int Input_Get_Key(PT_Input_Event ptInputEvent)
{
	pthread_mutex_lock(&g_tMutex);
	pthread_cond_wait(&g_tInputCond, &g_tMutex);
	
	*ptInputEvent = g_tInputEvent;

	pthread_mutex_unlock(&g_tMutex);

	return 0;
}

