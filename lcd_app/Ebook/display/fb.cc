#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>

struct fb_device
{
	int fb_fd;
	struct fb_var_screeninfo vinfo;
	unsigned long screen_size;
	unsigned int line_size;
	unsigned int pixel_size;
	char *fbp;
};

int fb_init(struct fb_device *fb_dev)
{
	fb_dev->fb_fd = open("/dev/fb0", O_RDWR);
	if(!fb_dev->fb_fd)
	{
		printf("Error:cannot open framebuffer device.\n");
		return -1;
	}

	if(ioctl(fb_dev->fb_fd, FBIOGET_VSCREENINFO, &fb_dev->vinfo))
	{
		printf("Error:get variable information error.\n");
		return -1;
	}

	printf("screen_bits_per_pixel = %d\n", fb_dev->vinfo.bits_per_pixel);
	printf("screen x size = %d\n", fb_dev->vinfo.xres);
	printf("screen y size = %d\n", fb_dev->vinfo.yres);

	fb_dev->pixel_size = fb_dev->vinfo.bits_per_pixel / 8;//pixel size in bytes
	fb_dev->line_size = fb_dev->vinfo.xres * fb_dev->pixel_size;//line size in bytes
	fb_dev->screen_size = fb_dev->line_size * fb_dev->vinfo.yres;//screen size in bytes
	printf("pixel_size = %d\n", fb_dev->pixel_size);
	printf("line_size = %d\n", fb_dev->line_size);
	printf("screen_size = %d\n", fb_dev->screen_size);
	
	//map the device to memory
	fb_dev->fbp = (char *)mmap(0, fb_dev->screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_dev->fb_fd, 0);
	if((int)fb_dev->fbp == -1)
	{
		printf("Error:fail to map framebuffer device to memory.\n");
	}

	return 0;
}

int fb_clean(struct fb_device *fb_dev)
{
	memset(fb_dev->fbp, 0, fb_dev->screen_size);
	return 0;
}

int fb_remove(struct fb_device *fb_dev)
{
	munmap(fb_dev->fbp, fb_dev->screen_size);
	close(fb_dev->fb_fd);
	return 0;
}
