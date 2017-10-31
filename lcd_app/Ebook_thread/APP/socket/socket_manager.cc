#include "socket_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "memwatch.h"

static PT_Socket_Opr g_ptSocketOprHead;

static pthread_mutex_t g_tSocketMutex; /* 互斥体lock 用于对缓冲区的互斥操作 */
static pthread_cond_t g_tSocketCond; /* 缓冲区非空的条件变量 */

char g_cSocketDataSend[DATA_MAX_LEN];

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
	iError = UDP_Socket_Init();
	if (iError) {
		printf("Error:udp init fail.\n");
		return -1;
	}

	iError = TCP_Socket_Init();
	if (iError) {
		printf("Error:tcp init fail.\n");
		return -1;
	}
	return 0;
}

static void *Socket_Thread_Function(void *arg)
{
	PT_Socket_Opr ptSocketOprTmp;
	
	ptSocketOprTmp = (PT_Socket_Opr)arg;

	while (1) {
		pthread_mutex_lock(&g_tSocketMutex);
		pthread_cond_wait(&g_tSocketCond, &g_tSocketMutex);
		if (ptSocketOprTmp->Socket_Send_Data(g_cSocketDataSend) <= 0) {
			printf("Error:socket send data error.\n");
		}
		pthread_mutex_unlock(&g_tSocketMutex);
	}
}

int All_Socket_Init(void)
{
	int iError = -1;
	int iRet;
	PT_Socket_Opr ptSocketOprTmp;

    pthread_mutex_init(&g_tSocketMutex, NULL);
    pthread_cond_init(&g_tSocketCond, NULL);

	if (!g_ptSocketOprHead) {
		iError = -1;
	} else {
		ptSocketOprTmp = g_ptSocketOprHead;
		while (ptSocketOprTmp) {
			iRet = ptSocketOprTmp->Socket_Init("192.168.0.3");
			if (0 == iRet) {
				printf("pthread_create: %s\n", ptSocketOprTmp->c_pcName);
				pthread_create(&ptSocketOprTmp->tTreadID, NULL, Socket_Thread_Function, (void *)ptSocketOprTmp);
				iError = 0;
			}
			ptSocketOprTmp = ptSocketOprTmp->ptNext;
		}
	}

	printf("All_Socket_Init over.\n");

	return iError;
}

int Socket_Send(char *pcDataSend)
{
	int iDataSendLen = strlen(pcDataSend);
	if (iDataSendLen > DATA_MAX_LEN) {
		printf("Error:sending data too much.");
		return -1;
	}
	
	pthread_mutex_lock(&g_tSocketMutex);

	strcpy(g_cSocketDataSend, pcDataSend);
	
	pthread_cond_signal(&g_tSocketCond);
	pthread_mutex_unlock(&g_tSocketMutex);
	
	return 0;
}

