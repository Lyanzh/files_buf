#ifndef __FONT_MANAGER__
#define __FONT_MANAGER__

struct font_device
{
	const char * font_name;
	int (*font_init)(char *font_filename, unsigned int font_size);
	//int (*disp_device_clean)(void);
	//void (*disp_device_put_pixel)(int, int, unsigned int);
	//int (*disp_device_remove)(void);
}

#endif

