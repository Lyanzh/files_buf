#include "input_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "memwatch.h"

static PT_Socket_Opr g_ptSocketOprHead;

pthread_mutex_t g_tMutex; /* 互斥体lock 用于对缓冲区的互斥操作 */
pthread_cond_t g_tInputCond; /* 缓冲区非空的条件变量 */

int Socket_Opr_Regisiter(PT_Socket_Opr ptSocketOpr)
{
	PT_Socket_Opr ptSocketOprTmp;
	
	if (!g_ptSocketOprHead) {
		g_ptSocketOprHead = ptSocketOpr;
	} else {
		ptSocketOprTmp = g_ptSocketOprHead;
		while (ptSocketOprTmp->ptNext) {
			ptSocketOprTmp = ptSocketOprTmp->ptNext;
		}
		ptSocketOprTmp->ptNext = ptSocketOpr;
	}
	ptSocketOpr->ptNext = NULL;

	return 0;
}

void Show_Socket_Opr(void)
{
	int i = 0;
	PT_Socket_Opr ptSocketOprTmp = g_ptSocketOprHead;
	while (ptSocketOprTmp) {
		printf("%d: %s\n", i++, ptSocketOprTmp->c_pcName);
		ptSocketOprTmp = ptSocketOprTmp->ptNext;
	}
}

PT_Socket_Opr Get_Socket_Opr(char *pcName)
{
	PT_Socket_Opr ptSocketOprTmp = g_ptSocketOprHead;
	while (ptSocketOprTmp) {
		if (strcmp(ptSocketOprTmp->c_pcName, pcName) == 0) {
			printf("get Socket %s.\n", pcName);
			return ptSocketOprTmp;
		} else {
			ptSocketOprTmp = ptSocketOprTmp->ptNext;
		}
	}
	printf("Error:can't get Socket %s.\n", pcName);
	return NULL;
}

int Socket_Opr_Init(void)
{
	int iError;
	iError = Stdin_Input_Init();
	if (iError) {
		printf("Error:Stdin init fail.\n");
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
	} else if (ptInputEvent->iType == INPUT_TYPE_BUTTON) {
		if (ptInputData->cCode == 0x01) {
			ptInputEvent->iVal = INPUT_VALUE_UP;
		} else if (ptInputData->cCode == 0x02) {
			ptInputEvent->iVal = INPUT_VALUE_DOWN;
		} else if (ptInputData->cCode == 0x03) {
			ptInputEvent->iVal = INPUT_VALUE_EXIT;
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


