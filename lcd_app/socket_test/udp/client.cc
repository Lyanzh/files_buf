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
	socklen_t peer_addr_size;

	int iRecvLen;
	char cDataSend[DATA_MAX_LEN];
	char cDataRecv[DATA_MAX_LEN];

	if (argc != 2) {
		printf("usage: ./client <ipaddress>\n");
		return -1;
	}
	
	iClientSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (iClientSocketFd == -1) {
		perror("Create socket error");
		return -1;
	}

	memset(&tServerAddr, 0, sizeof(struct sockaddr_in));
	tServerAddr.sin_family = AF_INET;
	tServerAddr.sin_port = htons(DEFAULT_PORT);
	tServerAddr.sin_addr.s_addr = inet_addr(argv[1]);

	while (1) {
		printf("send msg to server %s:", argv[1]);
		fgets(cDataSend, DATA_MAX_LEN, stdin);

		if (sendto(iClientSocketFd, cDataSend, strlen(cDataSend), 0,
        		(struct sockaddr *)&tServerAddr, sizeof(struct sockaddr_in)) < 0) {
			perror("socket send error");
			//continue;
		}

		peer_addr_size = sizeof(struct sockaddr_in);
		iRecvLen = recvfrom(iClientSocketFd, cDataRecv, DATA_MAX_LEN, 0,
				(struct sockaddr *)&tServerAddr, &peer_addr_size);
		if (iRecvLen < 0) {
			perror("socket recv error");
			//continue;
		}

		cDataRecv[iRecvLen] = '\0';
		printf("recv data:%s\n", cDataRecv);
	}
	close(iClientSocketFd);
	return 0;
}

