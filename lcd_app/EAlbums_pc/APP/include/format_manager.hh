#ifndef _FORMAT_MANAGER_H
#define _FORMAT_MANAGER_H

#include "config.h"

typedef struct Picture_Region	//һ����ɫ�����������������ڲ�������
{
	char           *pcData;		//��ɫ�����׵�ַ
	unsigned short wBpp;		//������/������
	unsigned long  dwWidth;		//���ؿ��
	unsigned long  dwHeight;	//���ظ߶�
} T_PicRegion, *PT_PicRegion;

typedef struct Format_Operation
{
	const char * c_pcName;
	int iHeadLen;
	int (*Init)(char *pcFilePath);
	int (*isSupport)(unsigned char *pucBufHead);
	int (*Get_Pic_Region)(char *pcFilePath, PT_PicRegion ptPicRegion);
	struct Format_Operation *ptNext;
} T_Format_Opr, *PT_Format_Opr;

int BMP_Format_Init(void);
int JPEG_Format_Init(void);

extern int Format_Opr_Regisiter(PT_Format_Opr ptFormatOpr);
extern void Show_Format_Opr(void);
extern PT_Format_Opr Get_Format_Opr(char *pcName);
extern int Format_Opr_Init(void);

#endif


