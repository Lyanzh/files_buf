#ifndef _DRAW_H
#define _DRAW_H

#include "format_manager.h"
#include "disp_manager.h"

typedef struct Icon_Info
{
	char *pcName;
	int iX;
	int iY;
} T_IconInfo, *PT_IconInfo;

extern void Fb_Lcd_Show_Pic(int iX, int iY, PT_PicRegion ptPicReg);
extern void Pic_Zoom(PT_PicRegion ptDstPicReg, PT_PicRegion ptSrcPicReg);

#endif

