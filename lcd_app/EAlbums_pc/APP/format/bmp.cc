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

#pragma pack (1) /*ָ����1�ֽڶ���*/

typedef struct Bitmap_File_Head
{  
	unsigned short wFileType;	/* 2�ֽڣ��ļ����� */
	unsigned long  dwFilefSize;	/* 4�ֽڣ��ļ���С */
	unsigned short wReserved1;	/* 2�ֽڣ����� */
	unsigned short wReserved2;	/* 2�ֽڣ����� */
	unsigned long  dwOffBits;	/* 4�ֽڣ���ͷ��λͼ���ݵ�ƫ�� */
} T_Bitmap_File_Head, *PT_Bitmap_File_Head;

typedef struct Bitmap_Information
{
	unsigned long dwSize;			/* 4�ֽڣ���Ϣͷ�Ĵ�С */
	unsigned long dwWidth;			/* 4�ֽڣ�������Ϊ��λ˵��ͼ��Ŀ�� */
	long          lHeight;			/* 4�ֽڣ�������Ϊ��λ˵��ͼ��ĸ߶ȣ�
									 * ͬʱ���Ϊ����˵��λͼ�����������ݱ�ʾ��ͼ������½ǵ����Ͻǣ���
									 * ���Ϊ��˵������
									 */
	unsigned short wPlanes;			/* 2�ֽڣ�ΪĿ���豸˵����ɫƽ�������ܱ�����Ϊ1 */
	unsigned short wBitCount;		/* 2�ֽڣ�˵��������/��������ֵ��1��2��4��8��16��24��32 */
	unsigned long  dwCompression;	/* 4�ֽڣ�˵��ͼ���ѹ�����ͣ���õľ���0��BI_RGB������ʾ��ѹ�� */
	unsigned long  dwSizeImages;	/* 4�ֽڣ�˵��λͼ���ݵĴ�С������BI_RGB��ʽʱ����������Ϊ0 */
	unsigned long  dwXPelsPerMeter;	/* 4�ֽڣ���ʾˮƽ�ֱ��ʣ���λ������/�ף��з������� */
	unsigned long  dwYPelsPerMeter;	/* 4�ֽڣ���ʾ��ֱ�ֱ��ʣ���λ������/�ף��з������� */
	unsigned long  dwClrUsed;		/* 4�ֽڣ�˵��λͼʹ�õĵ�ɫ���е���ɫ��������Ϊ0˵��ʹ������ */
	unsigned long  dwClrImportant;	/* 4�ֽڣ�˵����ͼ����ʾ����ҪӰ�����ɫ��������Ϊ0˵������Ҫ */
} T_Bitmap_Info, *PT_Bitmap_Info;

#pragma pack () /*ȡ��ָ�����룬�ָ�ȱʡ����*/



static int BMP_Get_Pic_Region(char *pcFilePath, PT_PicRegion ptPicReg)
{
	int i;
	int iFd;
	struct stat tFileStat;
	PT_Bitmap_File_Head ptBitmapFileHead;
	PT_Bitmap_Info ptBitmapInfo;
	char *pcFileData;
	int iBitmapLineByteCnt;	/* ʵ��ÿ��λͼ��Ч���ֽ��� */
	int iFileLineByteCnt;	/* �����ÿ�е��ֽ��� */
	//int iImageDataSize;	/* ����λͼ�������Ĵ�С */

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

	/* BMPͼ��Ҫ��ÿ�е����ݵĳ��ȱ�����4�ı�����
	 * ���������Ҫ���б�����䣨��0��䣩���������Դﵽ���еĿ��ٴ�ȡ��
	 * ��ʱ��λͼ�������Ĵ�С��δ����(ͼƬ��xÿ�����ֽ���xͼƬ��)�ܱ�ʾ���ˣ���Ϊÿ�п��ܻ���Ҫ���б�����䡣
	 */
	/* �����ÿ�е��ֽ��� */
	iFileLineByteCnt = (((ptBitmapInfo->dwWidth * ptBitmapInfo->wBitCount) + 31) >> 5) << 2;
	/* �������ļ���λͼ�������Ĵ�СΪ */
	//iImageDataSize = iFileLineByteCnt * dwBitmapHeight;
	
	//printf("Line Count in Byte      = %d\n", iFileLineByteCnt);
	//printf("Image Data Size in Byte = %d\n", iImageDataSize);

	/* ʵ��ÿ��λͼ��Ч���ֽ��� */
	iBitmapLineByteCnt = ptPicReg->dwWidth * ptPicReg->wBpp / 8;

	ptPicReg->pcData = (char *)malloc(iBitmapLineByteCnt * ptPicReg->dwHeight);
	if (ptPicReg->pcData == NULL) {
		printf("Error:malloc ptPicReg->pcData error\n");
		return -1;
	}

	if (ptBitmapInfo->lHeight < 0) {	/* λͼ���� */
		for (i = 0; i < dwBitmapHeight; i++) {
			pcPicRegLineData = ptPicReg->pcData + iFileLineByteCnt * i;
			pcFileLineData = pcFileBitmapData + iFileLineByteCnt * i;
			memcpy(pcPicRegLineData, pcFileLineData, iBitmapLineByteCnt);
		}
	} else {	/* λͼ���� */
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

