//#include "format_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

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

typedef struct Picture_Region	//һ����ɫ�����������������ڲ�������
{
	char           *pcData;		//��ɫ�����׵�ַ
	unsigned short wBpp;		//������/������
	unsigned long  dwLineByteCnt;//һ�����ݵ�������(�ֽڿ��)��
								//abs(byte_width)�п��ܴ��ڵ���width*sizeof(TARGB32);
	unsigned long  dwWidth;		//���ؿ��
	long           lHeight;		//���ظ߶�
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

	/* BMPͼ��Ҫ��ÿ�е����ݵĳ��ȱ�����4�ı�����
	 * ���������Ҫ���б�����䣨��0��䣩���������Դﵽ���еĿ��ٴ�ȡ��
	 * ��ʱ��λͼ�������Ĵ�С��δ����(ͼƬ��xÿ�����ֽ���xͼƬ��)�ܱ�ʾ���ˣ���Ϊÿ�п��ܻ���Ҫ���б�����䡣
	 */
	/* �����ÿ�е��ֽ��� */
	iLineByteCnt = (((g_ptBitmapInfo->dwWidth * g_ptBitmapInfo->wBitCount) + 31) >> 5) << 2;
	/* ������λͼ�������Ĵ�СΪ */
	iImageDataSize = iLineByteCnt * g_ptBitmapInfo->lHeight;
	
	printf("Line Count in Byte      = %d\n", iLineByteCnt);
	printf("Image Data Size in Byte = %d\n", iImageDataSize);

	ptPicReg->dwLineByteCnt = iLineByteCnt;

	ptPicReg->pcData = (char *)malloc(iImageDataSize);

	if (g_ptBitmapInfo->lHeight < 0) {	/* λͼ���� */
		memcpy(ptPicReg->pcData, pcFileBitmapData, iImageDataSize);
	} else {	/* λͼ���� */
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
	for (dwIndexX = 0; dwIndexX < dwDstWidth; dwIndexX++)/* ���ɱ����д�����Ҫ��ԭͼ��ȡ�ĵ��x���� */
	{
		pdwSrcTableX[dwIndexX] = (dwIndexX * ptSrcPicReg->dwWidth / ptDstPicReg->dwWidth);
	}

	ptDstPicReg->pcData = (char *)malloc(ptDstPicReg->dwWidth * ptDstPicReg->lHeight * ptDstPicReg->wBpp);

	pcDstLineData = ptDstPicReg->pcData;
	for (dwIndexY = 0; dwIndexY < ptDstPicReg->lHeight; dwIndexY++)
	{
		dwSrcY = (dwIndexY * ptSrcPicReg->lHeight / ptDstPicReg->lHeight);/* ��Ҫ��ԭͼ��ȡ�ĵ��y���� */
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

