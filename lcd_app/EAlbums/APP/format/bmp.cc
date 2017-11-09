#include "format_manager.h"

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
	unsigned long dwHeight;			/* 4�ֽڣ�������Ϊ��λ˵��ͼ��ĸ߶ȣ�
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

static T_Format_Opr g_tBMPOpr = {
	.c_pcName   = "bmp",
};

/* BMPͼ��Ҫ��ÿ�е����ݵĳ��ȱ�����4�ı�����
 * ���������Ҫ���б�����䣨��0��䣩���������Դﵽ���еĿ��ٴ�ȡ��
 * ��ʱ��λͼ�������Ĵ�С��δ����(ͼƬ��xÿ�����ֽ���xͼƬ��)�ܱ�ʾ���ˣ���Ϊÿ�п��ܻ���Ҫ���б�����䡣
*/
/* �����ÿ�е��ֽ��� */
int iLineByteCnt = (((m_iImageWidth * m_iBitsPerPixel) + 31) >> 5) << 2;
/* ������λͼ�������Ĵ�СΪ */
m_iImageDataSize = iLineByteCnt * m_iImageHeight;

int BMP_Format_Init(void)
{
	return Format_Opr_Regisiter(&g_tBMPOpr);
}

