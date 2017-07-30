
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int fd;
	int ret;
    char val = 0;

    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0)
    {
        printf("error, can't open %s\n", "/dev/buttons");
        return 0;
    }

	while(1)
	{
		val = 0;
		ret = read(fd, &val, 1);
        if (ret < 0) {
			printf("read err!\n");
			continue;
		}

		if(val)
			printf("val = %d\n", val);
	}
    
    
    return 0;
}

