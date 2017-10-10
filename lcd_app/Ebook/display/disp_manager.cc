#include "disp_manager.h"
#include "fb_lcd.h"
#include <stdio.h>

static struct disp_device disp_dev = {
	.dev_name	  = "s3c2440-lcd",
	.attr.xres    = fb_dev_info.xres,
	.attr.yres    = fb_dev_info.yres,
	.dev_init     = fb_init,
	.clean_screen = fb_clean,
	.put_pixel    = lcd_put_pixel,
	.dev_remove   = fb_remove,
};

int disp_init(void)
{
	disp_dev.dev_init();
	disp_dev.clean_screen();
}

int disp_draw_bitmap()
{
	
}

