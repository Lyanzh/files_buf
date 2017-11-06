#include "socket_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "memwatch.h"

static PT_Socket_Opr g_ptSocketOprHead;

static pthread_mutex_t g_tSendMutex;
static pthread_cond_t g_tSendCond;

static pthread_mutex_t g_tRecvMutex;
static pthread_cond_t g_tRecvCond;

static int iIsSocketConnected;
char *g_pcSocketDataSend;
char  g_cSocketDataRecv[1024];

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

static void *Socket_Send_Thread_Function(void *arg)
{
	PT_Socket_Opr ptSocketOprTmp;
	
	ptSocketOprTmp = (PT_Socket_Opr)arg;

	while (1) {
		pthread_mutex_lock(&g_tSendMutex);
		pthread_cond_wait(&g_tSendCond, &g_tSendMutex);
		if (ptSocketOprTmp->Socket_Send_Data(g_pcSocketDataSend) <= 0) {
			printf("Error:socket send data error.\n");
		}
		free(g_pcSocketDataSend);
		pthread_mutex_unlock(&g_tSendMutex);
	}
}

static void *Socket_Recv_Thread_Function(void *arg)
{
	PT_Socket_Opr ptSocketOprTmp;
	
	ptSocketOprTmp = (PT_Socket_Opr)arg;

	while (1) {
		pthread_mutex_lock(&g_tRecvMutex);
		ptSocketOprTmp->Socket_Recv_Data(g_cSocketDataRecv);
		pthread_cond_signal(&g_tRecvCond);
		pthread_mutex_unlock(&g_tRecvMutex);
	}
}

int All_Socket_Init(void)
{
	int iError = -1;
	int iRet;
	PT_Socket_Opr ptSocketOprTmp;

    pthread_mutex_init(&g_tSendMutex, NULL);
    pthread_cond_init(&g_tSendCond, NULL);

    pthread_mutex_init(&g_tRecvMutex, NULL);
    pthread_cond_init(&g_tRecvCond, NULL);
    
	if (!g_ptSocketOprHead) {
		iError = -1;
	} else {
		ptSocketOprTmp = g_ptSocketOprHead;
		while (ptSocketOprTmp) {
			ptSocketOprTmp->iIsConnected = 0;
			iRet = ptSocketOprTmp->Socket_Init("192.168.0.3");
			if (0 == iRet) {
				ptSocketOprTmp->iIsConnected = 1;
				iIsSocketConnected = 1;
				printf("pthread_create: %s\n", ptSocketOprTmp->c_pcName);
				pthread_create(&ptSocketOprTmp->tSendTreadID, NULL,
					Socket_Send_Thread_Function, (void *)ptSocketOprTmp);
				pthread_create(&ptSocketOprTmp->tRecvTreadID, NULL,
					Socket_Recv_Thread_Function, (void *)ptSocketOprTmp);
				iError = 0;

				return iError;
			}
			ptSocketOprTmp = ptSocketOprTmp->ptNext;
		}
	}

	printf("All_Socket_Init over.\n");

	return iError;
}

int Socket_Send(char *pcDataSend)
{
	int iDataSendLen;

	if (!iIsSocketConnected) {
		printf("Socket has not connect yet.\n");
		return -1;
	}

	iDataSendLen = strlen(pcDataSend);
	if (iDataSendLen > DATA_MAX_LEN) {
		printf("Error:sending data too much!");
		return -1;
	} else if (iDataSendLen <= 0) {
		printf("Warning:no data to send.");
		return 0;
	}
	
	pthread_mutex_lock(&g_tSendMutex);

	g_pcSocketDataSend = (char *)malloc(iDataSendLen+1);

	strcpy(g_pcSocketDataSend, pcDataSend);
	
	pthread_cond_signal(&g_tSendCond);
	pthread_mutex_unlock(&g_tSendMutex);
	
	return 0;
}

