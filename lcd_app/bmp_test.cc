//#include "format_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#pragma pack (1) /*指定按1字节对齐*/

typedef struct Bitmap_File_Head
{  
	unsigned short wFileType;	/* 2字节，文件类型 */
	unsigned long  dwFilefSize;	/* 4字节，文件大小 */
	unsigned short wReserved1;	/* 2字节，保留 */
	unsigned short wReserved2;	/* 2字节，保留 */
	unsigned long  dwOffBits;	/* 4字节，从头到位图数据的偏移 */
} T_Bitmap_File_Head, *PT_Bitmap_File_Head;

typedef struct Bitmap_Information
{
	unsigned long dwSize;			/* 4字节，信息头的大小 */
	unsigned long dwWidth;			/* 4字节，以像素为单位说明图像的宽度 */
	long          lHeight;			/* 4字节，以像素为单位说明图像的高度，
									 * 同时如果为正，说明位图倒立（即数据表示从图像的左下角到右上角），
									 * 如果为负说明正向
									 */
	unsigned short wPlanes;			/* 2字节，为目标设备说明颜色平面数，总被设置为1 */
	unsigned short wBitCount;		/* 2字节，说明比特数/像素数，值有1、2、4、8、16、24、32 */
	unsigned long  dwCompression;	/* 4字节，说明图像的压缩类型，最常用的就是0（BI_RGB），表示不压缩 */
	unsigned long  dwSizeImages;	/* 4字节，说明位图数据的大小，当用BI_RGB格式时，可以设置为0 */
	unsigned long  dwXPelsPerMeter;	/* 4字节，表示水平分辨率，单位是像素/米，有符号整数 */
	unsigned long  dwYPelsPerMeter;	/* 4字节，表示垂直分辨率，单位是像素/米，有符号整数 */
	unsigned long  dwClrUsed;		/* 4字节，说明位图使用的调色板中的颜色索引数，为0说明使用所有 */
	unsigned long  dwClrImportant;	/* 4字节，说明对图像显示有重要影响的颜色索引数，为0说明都重要 */
} T_Bitmap_Info, *PT_Bitmap_Info;

#pragma pack () /*取消指定对齐，恢复缺省对齐*/

typedef struct Picture_Region	//一块颜色数据区的描述，便于参数传递
{
	char           *pcData;		//颜色数据首地址
	unsigned short wBpp;		//比特数/像素数
	//unsigned long  dwLineByteCnt;//一行数据的物理宽度(字节宽度)；
								//abs(byte_width)有可能大于等于width*sizeof(TARGB32);
	unsigned long  dwWidth;		//像素宽度
	unsigned long  dwHeight;	//像素高度
} T_PicRegion, *PT_PicRegion;

PT_Bitmap_File_Head g_ptBitmapFileHead;
PT_Bitmap_Info g_ptBitmapInfo;

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

int Fb_Init(void)
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

	//g_tDispDev.tDevAttr.dwXres  = g_ptFbDev->tFbVarInfo.xres;
	//g_tDispDev.tDevAttr.dwYres  = g_ptFbDev->tFbVarInfo.yres;
	//g_tDispDev.tDevAttr.dwBitsPerPixel = g_ptFbDev->tFbVarInfo.bits_per_pixel;
	
	//map the device to memory
	g_ptFbDev->pFbMem = (char *)mmap(0, g_ptFbDev->dwScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_ptFbDev->fb_fd, 0);
	if((int)g_ptFbDev->pFbMem == -1)
	{
		printf("Error:fail to map framebuffer device to memory.\n");
	}

	return 0;
}

void Fb_Lcd_Put_Pixel(int x, int y, unsigned int color)
{
	char *pen_8 = g_ptFbDev->pFbMem + g_ptFbDev->dwLineSize * y + g_ptFbDev->dwPixelSize * x;
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

#if 0
void Fb_Lcd_Show_Line(int iStartX, int iEndX, int iY, int iBpp, char *pcData)
{
	int iByte = iBpp / 8;
	int i = 0;
	int j = iStartX * iByte;
	unsigned int color;

	unsigned int red, green, blue, alph;

	//printf("pcData = %x\n", pcData[0]);

	for (i = iStartX, j = 0; i < iEndX; i++) {
		red = pcData[j];
		green = pcData[j+1];
		blue = pcData[j+2];
		alph = 0;
		color = ((red << 24) | (green << 16) | (blue << 8) | alph);
		//printf("color = %x\n", color);
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

int Fb_Clean(void)
{
	memset(g_ptFbDev->pFbMem, 0, g_ptFbDev->dwScreenSize);
	return 0;
}

int BMP_Get_Data(char *pcFilePath, PT_PicRegion ptPicReg)
{
	int i;
	int iFd;
	struct stat tFileStat;
	char *pcFileData;
	int iBitmapLineByteCnt;	/* 实际每行位图有效的字节数 */
	int iFileLineByteCnt;	/* 填充后的每行的字节数 */
	int iImageDataSize;		/* 填充后位图数据区的大小 */

	char *pcFileBitmapData;
	char *pcFileLineData;
	char *pcPicRegLineData;

	unsigned long dwBitmapHeight;

	iFd = open(pcFilePath, O_RDONLY);
	if (iFd < 0) {
		printf("Error:can not open %s\n", pcFilePath);
		return -1;
	}

	if (fstat(iFd, &tFileStat)) {
		printf("Error:get file stat error\n");
		close(iFd);
		return -1;
	}

	pcFileData = (char *)mmap(NULL, tFileStat.st_size, PROT_READ, MAP_SHARED, iFd, 0);
	if (pcFileData == (void *) -1) {
		printf("Error:map file error\n");
		close(iFd);
		return -1;
	}

	g_ptBitmapFileHead = (PT_Bitmap_File_Head)pcFileData;
	printf("File Type       = %x\n", g_ptBitmapFileHead->wFileType);
	printf("File total Size = %ld\n", g_ptBitmapFileHead->dwFilefSize);
	
	g_ptBitmapInfo = (PT_Bitmap_Info)(pcFileData+sizeof(T_Bitmap_File_Head));
	printf("Info Header Size        = %ld\n", g_ptBitmapInfo->dwSize);
	printf("Picture width in pixel  = %ld\n", g_ptBitmapInfo->dwWidth);
	printf("Picture height in pixel = %ld\n", g_ptBitmapInfo->lHeight);
	printf("Bit per pixel           = %d\n", g_ptBitmapInfo->wBitCount);

	dwBitmapHeight = abs(g_ptBitmapInfo->lHeight);

	ptPicReg->dwWidth  = g_ptBitmapInfo->dwWidth;
	ptPicReg->dwHeight = dwBitmapHeight;
	ptPicReg->wBpp     = g_ptBitmapInfo->wBitCount;

	pcFileBitmapData = pcFileData + g_ptBitmapFileHead->dwOffBits;

	/* BMP图像要求每行的数据的长度必须是4的倍数，
	 * 如果不够需要进行比特填充（以0填充），这样可以达到按行的快速存取。
	 * 这时，位图数据区的大小就未必是(图片宽x每像素字节数x图片高)能表示的了，因为每行可能还需要进行比特填充。
	 */
	/* 填充后的每行的字节数 */
	iFileLineByteCnt = (((g_ptBitmapInfo->dwWidth * g_ptBitmapInfo->wBitCount) + 31) >> 5) << 2;
	/* 这样，文件的位图数据区的大小为 */
	iImageDataSize = iFileLineByteCnt * dwBitmapHeight;
	
	printf("Line Count in Byte      = %d\n", iFileLineByteCnt);
	printf("Image Data Size in Byte = %d\n", iImageDataSize);

	//ptPicReg->dwLineByteCnt = iFileLineByteCnt;

	/* 实际每行位图有效的字节数 */
	iBitmapLineByteCnt = ptPicReg->dwWidth * ptPicReg->wBpp / 8;

	ptPicReg->pcData = (char *)malloc(iBitmapLineByteCnt * ptPicReg->dwHeight);

	if (g_ptBitmapInfo->lHeight < 0) {	/* 位图正向 */
		for (i = 0; i < dwBitmapHeight; i++) {
			pcPicRegLineData = ptPicReg->pcData + iFileLineByteCnt * i;
			pcFileLineData = pcFileBitmapData + iFileLineByteCnt * i;
			memcpy(pcPicRegLineData, pcFileLineData, iBitmapLineByteCnt);
		}
	} else {	/* 位图倒立 */
		for (i = 0; i < dwBitmapHeight; i++) {
			pcPicRegLineData = ptPicReg->pcData + iFileLineByteCnt * i;
			pcFileLineData = pcFileBitmapData + iFileLineByteCnt * (dwBitmapHeight - i);
			memcpy(pcPicRegLineData, pcFileLineData, iBitmapLineByteCnt);
		}
	}
	
	munmap(pcFileData, tFileStat.st_size);
	close(iFd);

	return 0;
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

void Pic_Merge()
{
	
}

T_PicRegion g_tDstPicReg;
T_PicRegion g_tSrcPicReg;

int main(int argc, char **argv)
{
	Fb_Init();
	Fb_Clean();

	if (BMP_Get_Data(argv[1], &g_tSrcPicReg)) {
		printf("Error:can not get bitmap\n");
		return -1;
	}

	Fb_Lcd_Show_Pic(0, 0, &g_tSrcPicReg);

	g_tDstPicReg.wBpp = 24;
	g_tDstPicReg.dwWidth = g_tSrcPicReg.dwWidth * 2;
	g_tDstPicReg.dwHeight= g_tSrcPicReg.dwHeight * 2;

	Pic_Zoom(&g_tDstPicReg, &g_tSrcPicReg);

	Fb_Lcd_Show_Pic(200, 200, &g_tDstPicReg);
	
	free(g_tSrcPicReg.pcData);
	free(g_tDstPicReg.pcData);
	return 0;
}

