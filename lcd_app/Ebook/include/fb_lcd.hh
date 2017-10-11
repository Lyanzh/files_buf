#ifndef __FB_LCD__
#define __FB_LCD__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/fb.h>


extern int fb_init(void);
extern int fb_clean(void);
extern int fb_remove(void);
extern void lcd_put_pixel(int x, int y, unsigned int color);

#endif

