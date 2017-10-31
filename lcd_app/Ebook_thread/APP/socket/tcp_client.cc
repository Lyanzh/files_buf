#include "socket_manager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define DEFAULT_PORT	8000

static int g_iClientSocketFd;

static int TCP_Init(char *pcServerAddr)
{
	struct sockaddr_in tServerAddr;
	
	g_iClientSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (g_iClientSocketFd == -1) {
		perror("Create socket error");
		return -1;
	}

	memset(&tServerAddr, 0, sizeof(struct sockaddr_in));
	tServerAddr.sin_family = AF_INET;
	tServerAddr.sin_port = htons(DEFAULT_PORT);
	tServerAddr.sin_addr.s_addr = inet_addr(pcServerAddr);

	if (connect(g_iClientSocketFd, (struct sockaddr *)&tServerAddr,
		sizeof(struct sockaddr_in)) == -1) {
		perror("socket connect error");
		close(g_iClientSocketFd);
		return -1;
	}
	return 0;
}

static int TCP_Send_Data(char *pcDataSend)
{
	int iDataSendLen = strlen(pcDataSend);
	if (iDataSendLen <= 0) {
		printf("no data to send.\n");
		return -1;
	}
	if (send(g_iClientSocketFd, pcDataSend, iDataSendLen, 0) < 0) {
		perror("socket send error");
		return -1;
	}
	return iDataSendLen;
}

static int TCP_Recv_Data(char *pcDataRecv)
{
	int iRecvLen;
	iRecvLen = recv(g_iClientSocketFd, pcDataRecv, DATA_MAX_LEN, 0);
	if (iRecvLen < 0) {
		perror("socket recv error");
		return -1;
	}
	pcDataRecv[iRecvLen] = '\0';
	return iRecvLen;
}

static void TCP_Exit(void)
{
	close(g_iClientSocketFd);
}

static T_Socket_Opr g_tTCPOpr = {
	.c_pcName    = "tcp",
	.Socket_Init = TCP_Init,
	.Socket_Exit = TCP_Exit,
	.Socket_Send_Data = TCP_Send_Data,
	.Socket_Recv_Data = TCP_Recv_Data,
};

int TCP_Socket_Init(void)
{
	return Socket_Opr_Regisiter(&g_tTCPOpr);
}
