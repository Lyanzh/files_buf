#include "draw.h"
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
			g_ptDispOprSelected->Put_Pixel((iX + x), (iY + y), color);
		}
	}
}

void Pic_Zoom(PT_PicRegion ptDstPicReg, PT_PicRegion ptSrcPicReg)
{
	unsigned long dwIndexX;
	unsigned long dwIndexY;

	unsigned long dwSrcLineByteCnt;
	unsigned long dwDstLineByteCnt;
	unsigned long *pdwSrcTableX;
	unsigned long dwSrcY;

	char *pcSrcLineData;
	char *pcDstLineData;
	
	if ((0 == ptDstPicReg->dwWidth) || (0 == ptDstPicReg->dwHeight) ||
		(0 == ptSrcPicReg->dwWidth) || (0 == ptSrcPicReg->dwHeight)) {
		printf("Error:please indicate the size to zoom\n");
		return;
	}

	if (ptDstPicReg->wBpp != ptSrcPicReg->wBpp) {
		printf("Error:can not zoom in this bpp %d\n", ptDstPicReg->wBpp);
		return;
	}

	printf("src Picture width in pixel  = %ld\n", ptSrcPicReg->dwWidth);
	printf("src Picture height in pixel = %ld\n", ptSrcPicReg->dwHeight);
	printf("src Bit per pixel           = %d\n", ptSrcPicReg->wBpp);
	printf("dst Picture width in pixel  = %ld\n", ptDstPicReg->dwWidth);
	printf("dst Picture height in pixel = %ld\n", ptDstPicReg->dwHeight);
	printf("dst Bit per pixel           = %d\n", ptDstPicReg->wBpp);

	dwSrcLineByteCnt = ptSrcPicReg->dwWidth * ptSrcPicReg->wBpp / 8;
	dwDstLineByteCnt = ptDstPicReg->dwWidth * ptDstPicReg->wBpp / 8;
	
	pdwSrcTableX = (unsigned long *)malloc(sizeof(unsigned long) * ptDstPicReg->dwWidth);
	for (dwIndexX = 0; dwIndexX < ptDstPicReg->dwWidth; dwIndexX++)/* 生成表，表中存有需要在原图像取的点的x坐标 */
	{
		pdwSrcTableX[dwIndexX] = dwIndexX * ptSrcPicReg->dwWidth / ptDstPicReg->dwWidth;
	}

	ptDstPicReg->pcData = (char *)malloc(dwDstLineByteCnt * ptDstPicReg->dwHeight);

	pcDstLineData = ptDstPicReg->pcData;
	for (dwIndexY = 0; dwIndexY < ptDstPicReg->dwHeight; dwIndexY++)
	{
		dwSrcY = (dwIndexY * ptSrcPicReg->dwHeight / ptDstPicReg->dwHeight);/* 需要在原图像取的点的y坐标 */

		pcSrcLineData = ptSrcPicReg->pcData + dwSrcLineByteCnt * dwSrcY;

		for (dwIndexX = 0; dwIndexX < ptDstPicReg->dwWidth; dwIndexX++) {
			pcDstLineData[dwIndexX] = pcSrcLineData[pdwSrcTableX[dwIndexX]];
		}
        
		pcDstLineData += dwDstLineByteCnt;
	}

	free(pdwSrcTableX);
}

