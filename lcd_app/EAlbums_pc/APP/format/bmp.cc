#include "format_manager.h"
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



static int BMP_Get_Pic_Region(char *pcFilePath, PT_PicRegion ptPicReg)
{
	int i;
	int iFd;
	struct stat tFileStat;
	PT_Bitmap_File_Head ptBitmapFileHead;
	PT_Bitmap_Info ptBitmapInfo;
	char *pcFileData;
	int iBitmapLineByteCnt;	/* 实际每行位图有效的字节数 */
	int iFileLineByteCnt;	/* 填充后的每行的字节数 */
	//int iImageDataSize;	/* 填充后位图数据区的大小 */

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

	ptBitmapFileHead = (PT_Bitmap_File_Head)pcFileData;
	//printf("File Type       = %x\n", ptBitmapFileHead->wFileType);
	//printf("File total Size = %ld\n", ptBitmapFileHead->dwFilefSize);
	
	ptBitmapInfo = (PT_Bitmap_Info)(pcFileData+sizeof(T_Bitmap_File_Head));
	//printf("Info Header Size        = %ld\n", ptBitmapInfo->dwSize);
	//printf("Picture width in pixel  = %ld\n", ptBitmapInfo->dwWidth);
	//printf("Picture height in pixel = %ld\n", ptBitmapInfo->lHeight);
	//printf("Bit per pixel           = %d\n", ptBitmapInfo->wBitCount);

	dwBitmapHeight = abs(ptBitmapInfo->lHeight);

	ptPicReg->dwWidth  = ptBitmapInfo->dwWidth;
	ptPicReg->dwHeight = dwBitmapHeight;
	ptPicReg->wBpp     = ptBitmapInfo->wBitCount;

	pcFileBitmapData = pcFileData + ptBitmapFileHead->dwOffBits;

	/* BMP图像要求每行的数据的长度必须是4的倍数，
	 * 如果不够需要进行比特填充（以0填充），这样可以达到按行的快速存取。
	 * 这时，位图数据区的大小就未必是(图片宽x每像素字节数x图片高)能表示的了，因为每行可能还需要进行比特填充。
	 */
	/* 填充后的每行的字节数 */
	iFileLineByteCnt = (((ptBitmapInfo->dwWidth * ptBitmapInfo->wBitCount) + 31) >> 5) << 2;
	/* 这样，文件的位图数据区的大小为 */
	//iImageDataSize = iFileLineByteCnt * dwBitmapHeight;
	
	//printf("Line Count in Byte      = %d\n", iFileLineByteCnt);
	//printf("Image Data Size in Byte = %d\n", iImageDataSize);

	/* 实际每行位图有效的字节数 */
	iBitmapLineByteCnt = ptPicReg->dwWidth * ptPicReg->wBpp / 8;

	ptPicReg->pcData = (char *)malloc(iBitmapLineByteCnt * ptPicReg->dwHeight);
	if (ptPicReg->pcData == NULL) {
		printf("Error:malloc ptPicReg->pcData error\n");
		return -1;
	}

	if (ptBitmapInfo->lHeight < 0) {	/* 位图正向 */
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

static T_Format_Opr g_tBMPOpr = {
	.c_pcName   = "bmp",
	.Get_Pic_Region = BMP_Get_Pic_Region,
};

int BMP_Format_Init(void)
{
	return Format_Opr_Regisiter(&g_tBMPOpr);
}

