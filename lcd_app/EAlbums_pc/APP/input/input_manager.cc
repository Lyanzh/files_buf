#include "input_manager.h"
#include "disp_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "memwatch.h"

static PT_Input_Opr g_ptInputOprHead;
T_Input_Event g_tInputEvent;

static pthread_mutex_t g_tMutex; /* 互斥体lock 用于对缓冲区的互斥操作 */
static pthread_cond_t g_tInputCond; /* 缓冲区非空的条件变量 */

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

#if 0
	iError = Touchscreen_Input_Init();
	if (iError) {
		printf("Error:Touchscreen init fail.\n");
		return -1;
	}

	iError = Button_Input_Init();
	if (iError) {
		printf("Error:Button init fail.\n");
		return -1;
	}
#endif
	
	return 0;
}

static int Input_Get_InputEvent(PT_Input_Event ptInputEvent, PT_Input_Data ptInputData)
{
	static unsigned int dwPressure = 0;
	static unsigned int iRecX;
	int iThreshold;
	int iDelta;

	if (!ptInputEvent || !ptInputData)
		return -1;

	ptInputEvent->iType = ptInputData->iType;
	ptInputEvent->cCode= ptInputData->cCode;
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
		//printf("pressure = %d\n", ptInputData->dwPressure);
		//printf("x = %d\n", ptInputData->iX);
		//printf("y = %d\n", ptInputData->iY);
	#if 0 /* press */
		if (ptInputData->dwPressure == 1 && 0 == dwPressure) {
			//first press
			if (ptInputData->iY > (g_ptDispOprSelected->tDevAttr.dwYres / 3 * 2))
				ptInputEvent->iVal = INPUT_VALUE_DOWN;
			else if (ptInputData->iY < (g_ptDispOprSelected->tDevAttr.dwYres / 3))
				ptInputEvent->iVal = INPUT_VALUE_UP;
			else
				ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
			dwPressure = ptInputData->dwPressure;
		} else if (ptInputData->dwPressure == 0) {
			dwPressure = 0;
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		} else {
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		}
	#endif
		/* slide */
		if (ptInputData->dwPressure == 1 && 0 == dwPressure) {
			/* first press */
			dwPressure = 1;
			iRecX = ptInputData->iX;
			//ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
			return -1;
		} else if (ptInputData->dwPressure == 0) {
			if (dwPressure == 1) {
				dwPressure = 0;
				iDelta = ptInputData->iX - iRecX;
				iThreshold = g_ptDispOprSelected->tDevAttr.dwXres / 5;
				//printf("iDelta = %d\n", iDelta);
				//printf("iThreshold = %d\n", iThreshold);
				if (iDelta > iThreshold) {
					ptInputEvent->iVal = INPUT_VALUE_DOWN;
				} else if (iDelta < (0 - iThreshold)) {
					ptInputEvent->iVal = INPUT_VALUE_UP;
				} else {
					return -1;
				}
			} else {
				return -1;
			}
		} else {
			return -1;
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
		if (ptInputOprTmp->Input_Get_Data(&tInputData) && Input_Get_InputEvent(&g_tInputEvent, &tInputData) == 0) {
			pthread_mutex_lock(&g_tMutex);
			pthread_cond_signal(&g_tInputCond);
			pthread_mutex_unlock(&g_tMutex);
		}
	}
	return NULL;
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

