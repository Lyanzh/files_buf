//#include "format_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

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
	unsigned long  dwLineByteCnt;//一行数据的物理宽度(字节宽度)；
								//abs(byte_width)有可能大于等于width*sizeof(TARGB32);
	unsigned long  dwWidth;		//像素宽度
	long           lHeight;		//像素高度
} T_PicRegion, *PT_PicRegion;

PT_Bitmap_File_Head g_ptBitmapFileHead;
PT_Bitmap_Info g_ptBitmapInfo;

int BMP_Init()
{
	
}

int BMP_Get_Data(char *pcFilePath, PT_PicRegion ptPicReg)
{
	int i;
	int iFd;
	struct stat tFileStat;
	char *pcFileData;
	int iLineByteCnt;
	int iImageDataSize;

	char *pcFileBitmapData;
	char *pcFileLineData;
	char *pcPicRegLineData;

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

	ptPicReg->dwWidth = g_ptBitmapInfo->dwWidth;
	ptPicReg->lHeight = g_ptBitmapInfo->lHeight;
	ptPicReg->wBpp    = g_ptBitmapInfo->wBitCount;

	pcFileBitmapData = pcFileData + g_ptBitmapFileHead->dwOffBits;

	/* BMP图像要求每行的数据的长度必须是4的倍数，
	 * 如果不够需要进行比特填充（以0填充），这样可以达到按行的快速存取。
	 * 这时，位图数据区的大小就未必是(图片宽x每像素字节数x图片高)能表示的了，因为每行可能还需要进行比特填充。
	 */
	/* 填充后的每行的字节数 */
	iLineByteCnt = (((g_ptBitmapInfo->dwWidth * g_ptBitmapInfo->wBitCount) + 31) >> 5) << 2;
	/* 这样，位图数据区的大小为 */
	iImageDataSize = iLineByteCnt * g_ptBitmapInfo->lHeight;
	
	printf("Line Count in Byte      = %d\n", iLineByteCnt);
	printf("Image Data Size in Byte = %d\n", iImageDataSize);

	ptPicReg->dwLineByteCnt = iLineByteCnt;

	ptPicReg->pcData = (char *)malloc(iImageDataSize);

	if (g_ptBitmapInfo->lHeight < 0) {	/* 位图正向 */
		memcpy(ptPicReg->pcData, pcFileBitmapData, iImageDataSize);
	} else {	/* 位图倒立 */
		for (i = 0; i < g_ptBitmapInfo->lHeight; i++) {
			pcPicRegLineData = ptPicReg->pcData + iLineByteCnt * i;
			pcFileLineData = pcFileBitmapData + iLineByteCnt * (g_ptBitmapInfo->lHeight - i);
			memcpy(pcPicRegLineData, pcFileLineData, iLineByteCnt);
		}
	}
	
	munmap(pcFileData, tFileStat.st_size);
	close(iFd);

	return 0;
}

void Pic_Zoom(PT_PicRegion ptDstPicReg, PT_PicRegion ptSrcPicReg)
{
	unsigned long dwIndexX = 0;
	unsigned long dwIndexY = 0;
	unsigned long dwSrcY;

	unsigned long dwDstWidth;
	unsigned long *pdwSrcTableX;

	char *pcSrcLineData;
	char *pcDstLineData;
	
	if ((0 == ptDstPicReg->dwWidth) || (0 == ptDstPicReg->lHeight) ||
		(0 == ptSrcPicReg->dwWidth) || (0 == ptSrcPicReg->lHeight))
		return;

	if (ptDstPicReg->wBpp != ptSrcPicReg->wBpp) {
		printf("Error:can not zoom in this bpp %d\n", ptDstPicReg->wBpp);
		return;
	}
	
	dwDstWidth = ptDstPicReg->dwWidth;
	pdwSrcTableX = (unsigned long *)malloc(sizeof(unsigned long) * dwDstWidth);
	for (dwIndexX = 0; dwIndexX < dwDstWidth; dwIndexX++)/* 生成表，表中存有需要在原图像取的点的x坐标 */
	{
		pdwSrcTableX[dwIndexX] = (dwIndexX * ptSrcPicReg->dwWidth / ptDstPicReg->dwWidth);
	}

	ptDstPicReg->pcData = (char *)malloc(ptDstPicReg->dwWidth * ptDstPicReg->lHeight * ptDstPicReg->wBpp);

	pcDstLineData = ptDstPicReg->pcData;
	for (dwIndexY = 0; dwIndexY < ptDstPicReg->lHeight; dwIndexY++)
	{
		dwSrcY = (dwIndexY * ptSrcPicReg->lHeight / ptDstPicReg->lHeight);/* 需要在原图像取的点的y坐标 */
		pcSrcLineData = ptSrcPicReg->pcData + ptSrcPicReg->dwLineByteCnt * dwSrcY;
        
		for (dwIndexX = 0; dwIndexX < dwDstWidth; dwIndexX++)
			pcDstLineData[dwIndexX] = pcSrcLineData[pdwSrcTableX[dwIndexX]];
        
		pcDstLineData += ptDstPicReg->dwLineByteCnt;
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
	if (BMP_Get_Data(argv[1], &g_tSrcPicReg)) {
		printf("Error:can not get bitmap\n");
		return -1;
	}

	g_tDstPicReg.wBpp = 24;

	Pic_Zoom(&g_tDstPicReg, &g_tSrcPicReg);
	
	free(g_tSrcPicReg.pcData);
	free(g_tDstPicReg.pcData);
	return 0;
}

