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

static unsigned int Get_Pre_One_Bits(unsigned char pucBuf)
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
	int i;
	unsigned char pucBuf;
	unsigned int dwByteNum;
	unsigned int dwVal = 0;

	if (pucBufStart > pucBufEnd) {
		/* file end */
		return 0;
	}

	pucBuf = pucBufStart[0];
	dwByteNum = Get_Pre_One_Bits(pucBuf);

	if (dwByteNum == 0) {
		/* ASCII */
		*pdwCode = pucBufStart[0];
		return 1;
	} else {
		if ((pucBufStart + dwByteNum - 1) > pucBufEnd) {
			/* file end */
			return 0;
		}
		pucBuf = pucBuf << dwByteNum;
		pucBuf = pucBuf >> dwByteNum;
		dwVal += pucBuf;
		for (i = 1; i < dwByteNum; i++) {
			pucBuf =  pucBufStart[i] & 0x3f;
			dwVal = dwVal << 6;
			dwVal += pucBuf;
		}
		*pdwCode = dwVal;
		return dwByteNum;
	}
}

static T_Encoding_Opr g_tUtf8EncodingOpr = {
	.c_pEncodingName = "utf-8",
	.iHeadLen  = 3,
	.isSupport = isUtf8Coding,
	.Get_Code  = Utf8_Get_Code,
};

int Utf8_Encoding_Init(void)
{
	Add_Font_Opr_For_Encoding(&g_tUtf8EncodingOpr, Get_Font_Opr("freetype"));
	return Encoding_Opr_Regisiter(&g_tUtf8EncodingOpr);
}

