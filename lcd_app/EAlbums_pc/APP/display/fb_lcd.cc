#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include "disp_manager.h"

#include "memwatch.h"

static T_Disp_Opr g_tS3cDispOpr;

typedef struct FbDevice
{
	int iFd;
	struct fb_var_screeninfo tFbVarInfo;
	unsigned int dwScreenSize;
	unsigned int dwLineSize;
	unsigned int dwPixelSize;
	char *pcFbMem;
}T_FbDev, *PT_FbDev;

static PT_FbDev g_ptFbDev;

void Fb_Lcd_Put_Pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = (unsigned char *)(g_ptFbDev->pcFbMem + g_ptFbDev->dwLineSize * y + g_ptFbDev->dwPixelSize * x);
	unsigned short *pen_16;
	unsigned int *pen_32;
	
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;
	
	unsigned int red, green, blue;
	
	switch(g_ptFbDev->tFbVarInfo.bits_per_pixel)
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
		printf("cannot surport %dbpp\n", g_ptFbDev->tFbVarInfo.bits_per_pixel);
		break;
	}
}

static void Fb_Lcd_Pixel_Store(int x, int y, unsigned int color, char *pcMem)
{
	unsigned char *pen_8;
	unsigned short *pen_16;
	unsigned int *pen_32;

	if (x < g_ptFbDev->tFbVarInfo.xres && y < g_ptFbDev->tFbVarInfo.yres) {
		pen_8 = (unsigned char *)(pcMem + g_ptFbDev->dwLineSize * y
				+ g_ptFbDev->dwPixelSize * x);
		
		pen_16 = (unsigned short *)pen_8;
		pen_32 = (unsigned int *)pen_8;
		
		unsigned int red, green, blue;
		
		switch(g_ptFbDev->tFbVarInfo.bits_per_pixel)
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
			printf("cannot surport %dbpp\n", g_ptFbDev->tFbVarInfo.bits_per_pixel);
			break;
		}
	}
}

int Fb_Init(void)
{
	g_ptFbDev = (PT_FbDev)malloc(sizeof(T_FbDev));
	if(!g_ptFbDev)
	{
		printf("Error:cannot malloc device struct memery.\n");
	}

	g_ptFbDev->iFd = open("/dev/fb0", O_RDWR);
	if(!g_ptFbDev->iFd)
	{
		printf("Error:cannot open framebuffer device.\n");
		return -1;
	}

	if(ioctl(g_ptFbDev->iFd, FBIOGET_VSCREENINFO, &g_ptFbDev->tFbVarInfo))
	{
		printf("Error:get variable information error.\n");
		return -1;
	}

	printf("screen_bits_per_pixel = %d\n", g_ptFbDev->tFbVarInfo.bits_per_pixel);
	printf("screen x size = %d\n", g_ptFbDev->tFbVarInfo.xres);
	printf("screen y size = %d\n", g_ptFbDev->tFbVarInfo.yres);

	g_ptFbDev->dwPixelSize = g_ptFbDev->tFbVarInfo.bits_per_pixel / 8;//pixel size in bytes
	g_ptFbDev->dwLineSize = g_ptFbDev->tFbVarInfo.xres * g_ptFbDev->dwPixelSize;//line size in bytes
	g_ptFbDev->dwScreenSize = g_ptFbDev->dwLineSize * g_ptFbDev->tFbVarInfo.yres;//screen size in bytes
	printf("pixel_size = %d\n", g_ptFbDev->dwPixelSize);
	printf("line_size = %d\n", g_ptFbDev->dwLineSize);
	printf("screen_size = %d\n", g_ptFbDev->dwScreenSize);

	g_tS3cDispOpr.tDevAttr.dwXres  = g_ptFbDev->tFbVarInfo.xres;
	g_tS3cDispOpr.tDevAttr.dwYres  = g_ptFbDev->tFbVarInfo.yres;
	g_tS3cDispOpr.tDevAttr.dwBitsPerPixel = g_ptFbDev->tFbVarInfo.bits_per_pixel;
	g_tS3cDispOpr.dwScreenSize = g_ptFbDev->dwScreenSize;
	
	//map the device to memory
	g_ptFbDev->pcFbMem = (char *)mmap(0, g_ptFbDev->dwScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_ptFbDev->iFd, 0);
	if(g_ptFbDev->pcFbMem == MAP_FAILED)
	{
		printf("Error:fail to map framebuffer device to memory.\n");
	}

	g_tS3cDispOpr.pcMem = g_ptFbDev->pcFbMem;

	return 0;
}

int Fb_Clean(void)
{
	memset(g_ptFbDev->pcFbMem, 0, g_ptFbDev->dwScreenSize);
	return 0;
}

int Fb_Remove(void)
{
	munmap(g_ptFbDev->pcFbMem, g_ptFbDev->dwScreenSize);
	close(g_ptFbDev->iFd);
	Do_Free(g_ptFbDev);
	return 0;
}

static T_Disp_Opr g_tS3cDispOpr = {
	.pcName	  = "s3c2440-lcd",
	.Init     = Fb_Init,
	.Clean_Screen = Fb_Clean,
	.Put_Pixel    = Fb_Lcd_Put_Pixel,
	.Store_Pixel  = Fb_Lcd_Pixel_Store,
	.Dev_Remove   = Fb_Remove,
};

int Fb_Dev_Init(void)
{
	return Disp_Opr_Regisiter(&g_tS3cDispOpr);
}

