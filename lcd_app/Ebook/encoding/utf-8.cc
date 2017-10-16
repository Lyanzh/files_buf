#include "encoding_manager.h"
#include <string.h>

#include "memwatch.h"

/* return: 1->yes, 0->no */
static int isUtf8Coding(unsigned char *pucBufHead)
{
	const char aStrUtf8[] = {0xEF, 0xBB, 0xBF};

	if (strncmp((const char *)pucBufHead, aStrUtf8, 3) == 0) {
		/* utf-8 */
		return 1;
	} else {
		return 0;
	}
}

static unsigned int Utf8_Get_Byte_Num(unsigned char pucBuf)
{
	unsigned int i;
	unsigned int j = 0;
	for (i = 7; i >= 0; i--) {
		if ((pucBuf >> i) & 0x01)
			j++;
		else
			break;
	}
	return j;
}

/* return get code byte num */
static int Utf8_Get_Code(unsigned char *pucBufStart,
		unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	unsigned char *pucBuf = pucBufStart;
	unsigned char c = *pucBuf;

	unsigned int dwByteNum;

	dwByteNum = Utf8_Get_Byte_Num(c);

	//......

	return dwByteNum;
	
}

static T_Encoding_Opr g_tUtf8EncodingOpr = {
	.c_pEncodingName = "UTF-8",
	.iHeadLen  = 3,
	.isSupport = isUtf8Coding,
	.Get_Code  = Utf8_Get_Code,
};

int Utf8_Encoding_Init(void)
{
	Add_Font_Opr_For_Encoding(&g_tUtf8EncodingOpr, Get_Font_Opr("freetype"));
	return Encoding_Opr_Regisiter(&g_tUtf8EncodingOpr);
}
