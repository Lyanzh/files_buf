#include "draw.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "memwatch.h"

void Lcd_Show_Pic(int iX, int iY, PT_PicRegion ptPicReg)
{
	int x;
	int y;
	int iLine;
	int iWhich;
	int iByte;
	unsigned int color;
	unsigned int red, green, blue, alph;

	/* clean the area of pic show */
	//...

	iByte = ptPicReg->wBpp / 8;

	for (y = 0; y < ptPicReg->dwHeight; y++) {
		iLine = ptPicReg->dwWidth * iByte * y;
		for (x = 0; x < ptPicReg->dwWidth; x++) {
			iWhich = iLine + x * iByte;
			red   = ptPicReg->pcData[iWhich];
			green = ptPicReg->pcData[iWhich+1];
			blue  = ptPicReg->pcData[iWhich+2];
			alph  = 1;
			color = ((alph << 24) | (red << 16) | (green << 8) | (blue << 0));
			Selected_Display()->Put_Pixel((iX + x), (iY + y), color);
		}
	}
}

/*
 * Input:
 * 	iX : start x,
 * 	iY : start y,
 * 	ptPicReg : source data,
 * 	pcMem    : destination
 */
void Lcd_Merge(int iX, int iY, PT_PicRegion ptPicReg, char *pcMem)
{
	int x;
	int y;
	int iLine;
	int iWhich;
	int iByte;
	unsigned int color;
	unsigned int red, green, blue, alph;

	iByte = ptPicReg->wBpp / 8;

	for (y = 0; y < ptPicReg->dwHeight; y++) {
		iLine = ptPicReg->dwWidth * iByte * y;
		for (x = 0; x < ptPicReg->dwWidth; x++) {
			iWhich = iLine + x * iByte;
			red   = ptPicReg->pcData[iWhich];
			green = ptPicReg->pcData[iWhich+1];
			blue  = ptPicReg->pcData[iWhich+2];
			alph  = 1;
			color = ((alph << 24) | (red << 16) | (green << 8) | (blue << 0));
			Selected_Display()->Store_Pixel((iX + x), (iY + y), color, pcMem);
		}
	}
}

void Lcd_Mem_Flush(PT_Page_Mem ptPageMem)
{
	memcpy(Selected_Display()->pcMem, ptPageMem->pcMem, ptPageMem->dwMemSize);
}

void Pic_Zoom_Factor_For_Lcd(PT_PicRegion ptSrcPicReg, float *fFactor)
{
	float fFactorX;
	float fFactorY;

	if (!ptSrcPicReg) {
		printf("Warning:ptSrcPicReg invalided\n");
		return;
	}

	fFactorX = 1.0 * Selected_Display()->tDevAttr.dwXres / ptSrcPicReg->dwWidth;
	fFactorY = 1.0 * Selected_Display()->tDevAttr.dwYres / ptSrcPicReg->dwHeight;

	*fFactor = (fFactorX<=fFactorY?fFactorX:fFactorY);
	printf("Pic_Zoom_Factor_For_Lcd fFactor = %f\n", *fFactor);
}

/*
 * if fFactor is 0, please set ptDstPicReg
 */
void Pic_Zoom(PT_PicRegion ptDstPicReg, PT_PicRegion ptSrcPicReg, float fFactor)
{
	unsigned long dwIndexX;
	unsigned long dwIndexY;

	int i;

	unsigned short wSrcBppByte;
	unsigned short wDstBppByte;

	unsigned long dwSrcLineByteCnt;
	unsigned long dwDstLineByteCnt;
	unsigned long *pdwSrcTableX;
	unsigned long dwSrcY;

	char *pcSrcLineData;
	char *pcDstLineData;

	if ((0 == ptSrcPicReg->dwWidth) || (0 == ptSrcPicReg->dwHeight)) {
		printf("Error:size error\n");
		return;
	}

#if 0
	if ((0 == ptDstPicReg->dwWidth) || (0 == ptDstPicReg->dwHeight)) {
		ptDstPicReg->dwWidth = ptSrcPicReg->dwWidth * DEFAULT_ZOOM_FACTOR;
		ptDstPicReg->dwHeight = ptSrcPicReg->dwHeight * DEFAULT_ZOOM_FACTOR;
	}

	if (ptDstPicReg->wBpp == 0) {
		ptDstPicReg->wBpp = ptSrcPicReg->wBpp;
	}
#endif

	ptDstPicReg->wBpp = ptSrcPicReg->wBpp;

	if (fFactor > 0.01) {
		ptDstPicReg->dwWidth = ptSrcPicReg->dwWidth * fFactor;
		ptDstPicReg->dwHeight = ptSrcPicReg->dwHeight * fFactor;
	} else {
		if ((0 == ptDstPicReg->dwWidth) || (0 == ptDstPicReg->dwHeight)) {
			printf("Error:please indicate the size to zoom\n");
			return;
		}
	}

	if (ptDstPicReg->wBpp != ptSrcPicReg->wBpp) {
		printf("Error:can not zoom in this bpp %d\n", ptDstPicReg->wBpp);
		return;
	}

#if 0
	printf("src Picture width in pixel  = %ld\n", ptSrcPicReg->dwWidth);
	printf("src Picture height in pixel = %ld\n", ptSrcPicReg->dwHeight);
	printf("src Bit per pixel           = %d\n", ptSrcPicReg->wBpp);
	printf("dst Picture width in pixel  = %ld\n", ptDstPicReg->dwWidth);
	printf("dst Picture height in pixel = %ld\n", ptDstPicReg->dwHeight);
	printf("dst Bit per pixel           = %d\n", ptDstPicReg->wBpp);
#endif
	
	wSrcBppByte = ptSrcPicReg->wBpp / 8;
	wDstBppByte = ptDstPicReg->wBpp / 8;

	dwSrcLineByteCnt = ptSrcPicReg->dwWidth * wSrcBppByte;
	dwDstLineByteCnt = ptDstPicReg->dwWidth * wDstBppByte;

	//printf("dwDstLineByteCnt = %ld\n", dwDstLineByteCnt);
	
	pdwSrcTableX = (unsigned long *)malloc(sizeof(unsigned long) * ptDstPicReg->dwWidth);
	for (dwIndexX = 0; dwIndexX < ptDstPicReg->dwWidth; dwIndexX++)/* 生成表，表中存有需要在原图像取的点的x坐标 */
	{
		pdwSrcTableX[dwIndexX] = dwIndexX * ptSrcPicReg->dwWidth / ptDstPicReg->dwWidth;
	}

	ptDstPicReg->pcData = (char *)malloc(dwDstLineByteCnt * ptDstPicReg->dwHeight);
	if (ptDstPicReg->pcData == NULL) {
		printf("Error:malloc ptDstPicReg->pcData error\n");
		return;
	}
	printf("Pic_Zoom ptDstPicReg->pcData = 0x%x\n", ptDstPicReg->pcData);

#if 1
	pcDstLineData = ptDstPicReg->pcData;
	for (dwIndexY = 0; dwIndexY < ptDstPicReg->dwHeight; dwIndexY++)
	{
		dwSrcY = (dwIndexY * ptSrcPicReg->dwHeight / ptDstPicReg->dwHeight);/* 需要在原图像取的点的y坐标 */

		pcSrcLineData = ptSrcPicReg->pcData + dwSrcLineByteCnt * dwSrcY;

		for (dwIndexX = 0; dwIndexX < ptDstPicReg->dwWidth; dwIndexX++) {
			for (i = 0; i < wSrcBppByte; i++) {
				pcDstLineData[dwIndexX * wDstBppByte + i] = pcSrcLineData[pdwSrcTableX[dwIndexX] * wSrcBppByte  + i];
			}
		}
        
		pcDstLineData += dwDstLineByteCnt;
	}
#endif

	Do_Free(pdwSrcTableX);
}

