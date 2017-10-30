#include "socket_manager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define DEFAULT_PORT	8000
#define DATA_MAX_LEN	4096

static int g_iClientSocketFd;
static struct sockaddr_in g_tServerAddr;

static int UDP_Init(char *pcServerAddr)
{
	g_iClientSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (g_iClientSocketFd == -1) {
		perror("Create socket error");
		return -1;
	}

	memset(&g_tServerAddr, 0, sizeof(struct sockaddr_in));
	g_tServerAddr.sin_family = AF_INET;
	g_tServerAddr.sin_port = htons(DEFAULT_PORT);
	g_tServerAddr.sin_addr.s_addr = inet_addr(pcServerAddr);

	return 0;
}

static void UDP_Exit(void)
{
	close(g_iClientSocketFd);
}

static int UDP_Send_Data(char *pcDataSend)
{
	int iDataSendLen = strlen(pcDataSend);
	if (iDataSendLen <= 0) {
		printf("no data to send.\n");
		return -1;
	}
	
	if (sendto(g_iClientSocketFd, pcDataSend, iDataSendLen, 0,
		(struct sockaddr *)&g_tServerAddr, sizeof(struct sockaddr_in)) < 0) {
		perror("socket send error");
		return -1;
	}

	return iDataSendLen;
}

static int UDP_Recv_Data(char *pcDataRecv)
{
	int iRecvLen;
	socklen_t iServerAddrSize;
	
	peer_addr_size = sizeof(struct sockaddr_in);
	iRecvLen = recvfrom(g_iClientSocketFd, pcDataRecv, DATA_MAX_LEN, 0,
			(struct sockaddr *)&g_tServerAddr, &iServerAddrSize);
	if (iRecvLen < 0) {
		perror("socket recv error");
		return -1;
	}

	pcDataRecv[iRecvLen] = '\0';
	return iRecvLen;
}

static T_Socket_Opr g_tUDPOpr = {
	.c_pcName    = "udp",
	.Socket_Init = UDP_Init,
	.Socket_Exit = UDP_Exit,
	.Socket_Send_Data = UDP_Send_Data,
	.Socket_Recv_Data = UDP_Recv_Data,
};

int UDP_Socket_Init(void)
{
	return Socket_Opr_Regisiter(&g_tUDPOpr);
}

