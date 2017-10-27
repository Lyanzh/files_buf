#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define DEFAULT_PORT	8000
#define DATA_MAX_LEN	4096

int main(int argc, char **argv)
{
	int iClientSocketFd;
	struct sockaddr_in tServerAddr;

	int iRecvLen;
	char cDataSend[DATA_MAX_LEN];
	char cDataRecv[DATA_MAX_LEN];

	if (argc != 2) {
		printf("usage: ./client <ipaddress>\n");
		return -1;
	}
	
	iClientSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (iClientSocketFd == -1) {
		perror("Create socket error");
		return -1;
	}

	memset(&tServerAddr, 0, sizeof(struct sockaddr_in));
	tServerAddr.sin_family = AF_INET;
	tServerAddr.sin_port = htons(DEFAULT_PORT);
	if (inet_pton(AF_INET, argv[1], &tServerAddr.sin_addr) <= 0) {
		printf("inet_pton error for %s\n", argv[1]);
		close(iClientSocketFd);
		return -1;
	}

	if (connect(iClientSocketFd, (struct sockaddr *)&tServerAddr,
		sizeof(struct sockaddr_in)) == -1) {
		perror("socket connect error");
		close(iClientSocketFd);
		return -1;
	}

	while (1) {
		printf("send msg to server %s:", argv[1]);
		fgets(cDataSend, DATA_MAX_LEN, stdin);
		if (send(iClientSocketFd, cDataSend, strlen(cDataSend), 0) < 0) {
			printf("socket send error\n");
			continue;
		}

	#if 1
		iRecvLen = recv(iClientSocketFd, cDataRecv, DATA_MAX_LEN, 0);
		if (iRecvLen < 0) {
			printf("socket recv error\n");
			continue;
		}
		cDataRecv[iRecvLen] = '\0';
		printf("recv data:%s\n", cDataRecv);
	#endif
	}
	close(iClientSocketFd);
	return 0;
}
