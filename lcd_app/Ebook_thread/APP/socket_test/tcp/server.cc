#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
	int iServerSocketFd;
	iServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (iServerSocketFd == -1)
		return -1;

	int bind(iServerSocketFd, const struct sockaddr *addr, socklen_t addrlen);
}

