#ifndef _DRAW_H
#define _DRAW_H
#include "format_manager.h"
#include "disp_manager.h"
#include "page_manager.h"

typedef struct Icon_Info
{
	char *pcName;
	int iTopLeftX;
	int iTopLeftY;
	int iBottomRightX;
	int iBottomRightY;
} T_IconInfo, *PT_IconInfo;

typedef struct Picture_Curtain
{
	int iX;
	int iY;
	unsigned long  dwWidth;
	unsigned long  dwHeight;
} T_PicCurtain, *PT_PicCurtain;

extern void Lcd_Show_Pic(int iX, int iY, PT_PicRegion ptPicReg);
extern void Lcd_Merge(int iX, int iY, PT_PicRegion ptPicReg, char *pcMem);
extern void Lcd_Mem_Flush(PT_Page_Mem ptPageMem);
extern void Pic_Zoom(PT_PicRegion ptDstPicReg, PT_PicRegion ptSrcPicReg, float fFactor);
extern void Pic_Zoom_Factor_For_Lcd(PT_PicRegion ptSrcPicReg, float *fFactor);
extern int Lcd_Pic_Pos(PT_PicCurtain ptPagePicCurtain,
				int *iPicPosX, int *iPicPosY, PT_PicRegion ptPicReg);
#endif

