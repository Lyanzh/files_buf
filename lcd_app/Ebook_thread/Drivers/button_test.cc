#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int iFd;
	char cKeyVal;
	int iReadBytes;
	
	iFd = open("/dev/buttons", O_RDONLY);
	if (iFd < 0) {
		perror("open error");
		return -1;
	}

	while (1) {
		iReadBytes = read(iFd, &cKeyVal, 1);
		if (iReadBytes > 0) {
			printf("Key val: %x", cKeyVal);
		}
	}

	close(iFd);

	return 0;
}

