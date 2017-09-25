#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
	int fbfd = 0;
	struct fb_var_screeninfo vinfo;
	unsigned long screensize = 0;
	unsigned int screen_bits_per_pixel = 0;
	char *fbp;
	
	fbfd = open("/dev/fb0", O_RDWR);
	if(!fbfd)
	{
		printf("Error:cannot open framebuffer device.\n");
		return -1;
	}

	if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("Error:get variable information error.\n");
		return -1;
	}

	screen_bits_per_pixel = vinfo.bits_per_pixel;
	printf("screen_bits_per_pixel = %d\n", screen_bits_per_pixel);

	//size of the screen in bytes
	screensize = vinfo.xres * vinfo.yres * screen_bits_per_pixel / 8;

	//map the device to memory
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if((int)fbp == -1)
	{
		printf("Error:fail to map framebuffer device to memory.\n");
	}

	munmap(fbp, screensize);
	close(fbfd);
	return 0;
}