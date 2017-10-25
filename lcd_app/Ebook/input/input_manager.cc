#include "input_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "memwatch.h"

static PT_Input_Opr g_ptInputOprHead;

static fd_set g_tRFds;
static int g_iInputFdMax = -1;

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

	FD_ZERO(&g_tRFds);

	if (!g_ptInputOprHead) {
		iError = -1;
	} else {
		ptInputOprTmp = g_ptInputOprHead;
		while (ptInputOprTmp) {
			iRet = ptInputOprTmp->Input_Init();
			if (0 == iRet) {
				printf("ptInputOprTmp->iFd = %d\n", ptInputOprTmp->iFd);
				/* Watch ptInputOprTmp->iFd to see when it has input. */
				FD_SET(ptInputOprTmp->iFd, &g_tRFds);
				printf("FD_SET over\n");

				if (g_iInputFdMax < ptInputOprTmp->iFd)
					g_iInputFdMax = ptInputOprTmp->iFd;

				iError = 0;
			}

			ptInputOprTmp = ptInputOprTmp->ptNext;
		}

		g_iInputFdMax++;
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

int Input_Get_Key(PT_Input_Event ptInputEvent)
{
	struct timeval tv;
	fd_set tRFds;
	int retval;
   
	PT_Input_Opr ptInputOprTmp;
	T_Input_Data tInputData;
	
	ptInputOprTmp = g_ptInputOprHead;

	/* Wait up to five seconds. */
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	tRFds = g_tRFds;

	retval = select(g_iInputFdMax, &tRFds, NULL, NULL, &tv);

	if (retval == -1) {
		return -1;
	}

	if (retval > 0) {
		while (ptInputOprTmp) {
			if (FD_ISSET(ptInputOprTmp->iFd, &g_tRFds)) {
				if (ptInputOprTmp->Input_Get_Data(&tInputData)) {
					if (0 == Input_Get_InputEvent(ptInputEvent, &tInputData))
						return 1;
					else
						return -1;
				} else {
					return 0;
				}
			} 
			ptInputOprTmp = ptInputOprTmp->ptNext;
		}
	}

	return 0;
}

