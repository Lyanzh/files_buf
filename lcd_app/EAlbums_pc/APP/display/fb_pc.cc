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

static T_DispDev g_tDispDev;

typedef struct FbDevice
{
	int fb_fd;
	struct fb_var_screeninfo tFbVarInfo;
	unsigned int dwScreenSize;
	unsigned int dwLineSize;
	unsigned int dwPixelSize;
	char *pFbMem;
}T_FbDev, *PT_FbDev;

static PT_FbDev g_ptFbDev;

static void Fb_Lcd_Put_Pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = g_ptFbDev->pFbMem + g_ptFbDev->dwLineSize * y + g_ptFbDev->dwPixelSize * x;
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

#if 1
void Fb_Lcd_Show_Line(int iStartX, int iEndX, int iY, int iBpp, char *pcData)
{
	int iByte = iBpp / 8;
	int i = 0;
	int j = iStartX * iByte;
	unsigned int color;

	unsigned int red, green, blue, alph;

	for (i = iStartX, j = 0; i < iEndX; i++) {
		red = pcData[j];
		green = pcData[j+1];
		blue = pcData[j+2];
		alph = 0;
		color = ((red << 24) | (green << 16) | (blue << 8) | alph);
		j += iByte;
		Fb_Lcd_Put_Pixel(i, iY, color);
	}
}
#endif

void Fb_Lcd_Show_Pic(int iX, int iY, PT_PicRegion ptPicReg)
{
	int x;
	int y;
	int iLine;
	int iWhich;
	int byte;
	unsigned int color;
	unsigned int red, green, blue, alph;

	byte = ptPicReg->wBpp / 8;

	for (y = 0; y < ptPicReg->dwHeight; y++) {
		iLine = ptPicReg->dwWidth * byte * y;
		for (x = 0; x < ptPicReg->dwWidth; x++) {
			iWhich = iLine + x * byte;
			red   = ptPicReg->pcData[iWhich];
			green = ptPicReg->pcData[iWhich+1];
			blue  = ptPicReg->pcData[iWhich+2];
			alph  = 0;
			color = ((red << 24) | (green << 16) | (blue << 8) | alph);
			Fb_Lcd_Put_Pixel((iX + x), (iY + y), color);
		}
	}
}

static int Fb_Init(void)
{
	g_ptFbDev = (PT_FbDev)malloc(sizeof(T_FbDev));
	if(!g_ptFbDev)
	{
		printf("Error:cannot malloc device struct memery.\n");
	}

	g_ptFbDev->fb_fd = open("/dev/fb0", O_RDWR);
	if(!g_ptFbDev->fb_fd)
	{
		printf("Error:cannot open framebuffer device.\n");
		return -1;
	}

	if(ioctl(g_ptFbDev->fb_fd, FBIOGET_VSCREENINFO, &g_ptFbDev->tFbVarInfo))
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

	g_tDispDev.tDevAttr.dwXres  = g_ptFbDev->tFbVarInfo.xres;
	g_tDispDev.tDevAttr.dwYres  = g_ptFbDev->tFbVarInfo.yres;
	g_tDispDev.tDevAttr.dwBitsPerPixel = g_ptFbDev->tFbVarInfo.bits_per_pixel;
	
	//map the device to memory
	g_ptFbDev->pFbMem = (char *)mmap(0, g_ptFbDev->dwScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_ptFbDev->fb_fd, 0);
	if((int)g_ptFbDev->pFbMem == -1)
	{
		printf("Error:fail to map framebuffer device to memory.\n");
	}

	return 0;
}

static int Fb_Clean(void)
{
	memset(g_ptFbDev->pFbMem, 0, g_ptFbDev->dwScreenSize);
	return 0;
}

static int Fb_Remove(void)
{
	munmap(g_ptFbDev->pFbMem, g_ptFbDev->dwScreenSize);
	close(g_ptFbDev->fb_fd);
	free(g_ptFbDev);
	return 0;
}

static T_DispDev g_tPcDispDev = {
	.c_pDevName	  = "ubuntu",
	.Dev_Init     = Fb_Init,
	.Clean_Screen = Fb_Clean,
	.Put_Pixel    = Fb_Lcd_Put_Pixel,
	.Dev_Remove   = Fb_Remove,
};

int PC_Disp_Dev_Init(void)
{
	return Disp_Dev_Regisiter(&g_tPcDispDev);
}

