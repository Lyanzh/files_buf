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

void Fb_Lcd_Show_Pic(int iX, int iY, PT_PicRegion ptPicReg)
{
	int x;
	int y;
	int iLine;
	int iWhich;
	int iByte;
	unsigned int color;
	unsigned int red, green, blue, alph;

	/* clean the area of pic show */
	

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
			g_ptDispOprSelected->Put_Pixel((iX + x), (iY + y), color);
		}
	}
}

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
	ptDstPicReg->dwWidth = ptSrcPicReg->dwWidth * fFactor;
	ptDstPicReg->dwHeight = ptSrcPicReg->dwHeight * fFactor;
	ptDstPicReg->wBpp = ptSrcPicReg->wBpp;

#if 0
	if ((0 == ptDstPicReg->dwWidth) || (0 == ptDstPicReg->dwHeight) ||
		(0 == ptSrcPicReg->dwWidth) || (0 == ptSrcPicReg->dwHeight)) {
		printf("Error:please indicate the size to zoom\n");
		return;
	}
#endif

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
	for (dwIndexX = 0; dwIndexX < ptDstPicReg->dwWidth; dwIndexX++)/* ���ɱ����д�����Ҫ��ԭͼ��ȡ�ĵ��x���� */
	{
		pdwSrcTableX[dwIndexX] = dwIndexX * ptSrcPicReg->dwWidth / ptDstPicReg->dwWidth;
	}

	ptDstPicReg->pcData = (char *)malloc(dwDstLineByteCnt * ptDstPicReg->dwHeight);
	if (ptDstPicReg->pcData == NULL) {
		printf("Error:malloc ptDstPicReg->pcData error\n");
		return;
	}

	pcDstLineData = ptDstPicReg->pcData;
	for (dwIndexY = 0; dwIndexY < ptDstPicReg->dwHeight; dwIndexY++)
	{
		dwSrcY = (dwIndexY * ptSrcPicReg->dwHeight / ptDstPicReg->dwHeight);/* ��Ҫ��ԭͼ��ȡ�ĵ��y���� */

		pcSrcLineData = ptSrcPicReg->pcData + dwSrcLineByteCnt * dwSrcY;

		for (dwIndexX = 0; dwIndexX < ptDstPicReg->dwWidth; dwIndexX++) {
			for (i = 0; i < wSrcBppByte; i++) {
				pcDstLineData[dwIndexX * wDstBppByte + i] = pcSrcLineData[pdwSrcTableX[dwIndexX] * wSrcBppByte  + i];
			}
		}
        
		pcDstLineData += dwDstLineByteCnt;
	}

	free(pdwSrcTableX);
}

