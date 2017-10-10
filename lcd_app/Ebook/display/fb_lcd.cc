#include "include/fb_lcd.h"

struct fb_device
{
	int fb_fd;
	struct fb_var_screeninfo vinfo;
	unsigned long screen_size;
	unsigned int line_size;
	unsigned int pixel_size;
	char *fbp;
};

static struct fb_device *fb_dev;
struct fb_device_info fb_dev_info;

void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fb_dev->fbp + fb_dev->line_size * y + fb_dev->pixel_size * x;
	unsigned short *pen_16;
	unsigned int *pen_32;
	
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;
	
	unsigned int red, green, blue;
	
	switch(fb_dev->vinfo.bits_per_pixel)
	{
	case 8:
		*pen_8 = color;
		break;
	case 16:
		/* RGB565 */
		red = (color >> 16) & 0xff;
		green = (color >> 8) & 0xff;
		blue = (color >> 0) & 0xff;
		color = ((red >> 3) << 11) | ((green >> 2) << 5) | ((blue >> 3));
		*pen_16 = color;
		break;
	case 32:
		*pen_32 = color;
		break;
	default:
		printf("cannot surport %dbpp\n", fb_dev->vinfo.bits_per_pixel);
		break;
	}
}

int fb_init(void)
{
	fb_dev = (struct fb_device *)malloc(sizeof(struct fb_device));
	if(!fb_dev)
	{
		printf("Error:cannot malloc device struct memery.\n");
	}

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

	fb_dev_info.line_size = fb_dev->line_size;
	fb_dev_info.pixel_size = fb_dev->pixel_size;
	fb_dev_info.screen_size = fb_dev->screen_size;
	fb_dev_info.xres = fb_dev->vinfo.xres;
	fb_dev_info.yres = fb_dev->vinfo.yres;
	
	//map the device to memory
	fb_dev->fbp = (char *)mmap(0, fb_dev->screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_dev->fb_fd, 0);
	if((int)fb_dev->fbp == -1)
	{
		printf("Error:fail to map framebuffer device to memory.\n");
	}

	return 0;
}

int fb_clean(void)
{
	memset(fb_dev->fbp, 0, fb_dev->screen_size);
	return 0;
}

int fb_remove(void)
{
	munmap(fb_dev->fbp, fb_dev->screen_size);
	close(fb_dev->fb_fd);
	free(fb_dev);
	return 0;
}

