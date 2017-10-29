#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT	8000
#define LISTEN_BACKLOG	50
#define DATA_MAX_LEN	4096

int main(int argc, char **argv)
{
	int iServerSocketFd;
	struct sockaddr_in tServerAddr;
	struct sockaddr_in tClientAddr;
	socklen_t peer_addr_size;

	int iDataLen;
	char cDataBuf[DATA_MAX_LEN];
	char cClientAddrBuf[20];
	
	iServerSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (iServerSocketFd == -1) {
		perror("Create socket error");
		return -1;
	}

	memset(&tServerAddr, 0, sizeof(struct sockaddr_in));
	tServerAddr.sin_family = AF_INET;
	tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	tServerAddr.sin_port = htons(DEFAULT_PORT);
	if (bind(iServerSocketFd, (struct sockaddr *)&tServerAddr,
		sizeof(struct sockaddr_in)) == -1) {
		perror("socket bind error");
		return -1;
	}

    while (1) {
    	memset(&tClientAddr, 0, sizeof(struct sockaddr_in));
    	peer_addr_size = sizeof(struct sockaddr_in);
		iDataLen = recvfrom(iServerSocketFd, cDataBuf, DATA_MAX_LEN, 0,
				(struct sockaddr *)&tClientAddr, &peer_addr_size);
		if (iDataLen > 0) {
			if (!fork()) {
				cDataBuf[iDataLen] = '\0';
				inet_ntop(AF_INET, &tClientAddr.sin_addr.s_addr, cClientAddrBuf, peer_addr_size);
				printf("recv msg from client %s: %s\n", cClientAddrBuf, cDataBuf);

				if (sendto(iServerSocketFd, "thank you", strlen("thank you"), 0,
                		(struct sockaddr *)&tClientAddr, peer_addr_size) < 0) {
					perror("socket send error");
				}
				return 0;
			}
		}
	}
	close(iServerSocketFd);
	return 0;
}

