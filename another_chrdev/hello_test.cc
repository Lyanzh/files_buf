#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int fd;

	if(argc != 2)
		printf("dont do this\n");

	fd = open(argv[1], O_RDWR);
	if(fd < 0)
		printf("open %s error\n", argv[1]);
	else
		printf("open %s success\n", argv[1]);
	return 0;
}

