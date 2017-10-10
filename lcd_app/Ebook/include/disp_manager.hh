#ifndef __DISP_MANAGER__
#define __DISP_MANAGER__

struct dev_attr
{
	unsigned int xres;			/* visible resolution		*/
	unsigned int yres;
	unsigned int bits_per_pixel;
};

struct disp_device
{
	const char * dev_name;
	struct dev_attr attr;
	int (*dev_init)(void);
	int (*clean_screen)(void);
	void (*put_pixel)(int, int, unsigned int);
	int (*dev_remove)(void);
};

#endif
