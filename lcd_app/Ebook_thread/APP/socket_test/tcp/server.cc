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
	int iServerSocketFd, iClientSocketFd;
	struct sockaddr_in tServerAddr;
	struct sockaddr_in tClientAddr;
	socklen_t peer_addr_size;

	int iDataLen;
	char cDataBuf[DATA_MAX_LEN];
	char cClientAddrBuf[20];
	
	iServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (iServerSocketFd == -1) {
		printf("Create socket error\n");
		return -1;
	}

	memset(&tServerAddr, 0, sizeof(struct sockaddr_in));
	tServerAddr.sin_family = AF_INET;
	tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	tServerAddr.sin_port = htons(DEFAULT_PORT);
	if (bind(iServerSocketFd, (struct sockaddr *)&tServerAddr,
		sizeof(struct sockaddr_in)) == -1) {
		printf("socket bind error\n");
		return -1;
	}

    if (listen(iServerSocketFd, LISTEN_BACKLOG) == -1) {
		printf("socket listen error\n");
		return -1;
    }

    while (1) {
    	memset(&tClientAddr, 0, sizeof(struct sockaddr_in));
    	peer_addr_size = sizeof(struct sockaddr_in);
		iClientSocketFd = accept(iServerSocketFd, (struct sockaddr *)&tClientAddr, &peer_addr_size);
		if (iClientSocketFd != -1) {
			if (!fork()) {
				while (1) {
					iDataLen = recv(iClientSocketFd, cDataBuf, DATA_MAX_LEN, 0);
					if (iDataLen > 0) {
						cDataBuf[iDataLen] = '\0';
						inet_ntop(AF_INET, &tClientAddr.sin_addr.s_addr, cClientAddrBuf, peer_addr_size);
						printf("recv msg from client %s: %s\n", cClientAddrBuf, cDataBuf);

					#if 1
						if (send(iClientSocketFd, "thank you", strlen("thank you"), 0) < 0) {
							printf("socket send error\n");
							close(iClientSocketFd);
							continue;
						}
					#endif
					} else {
						close(iClientSocketFd);
						return -1;
					}
				}
			}
		}
		//close(iClientSocketFd);
	}
	close(iServerSocketFd);
	return 0;
}

