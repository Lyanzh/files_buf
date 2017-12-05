#include "encoding_manager.h"
#include <string.h>

//#include "memwatch.h"

/* return: 1->yes, 0->no */
static int isAsciiCoding(unsigned char *pucBufHead)
{
	const char aStrUtf8[] = {0xEF, 0xBB, 0xBF};
	const char aStrUtf16le[] = {0xFF, 0xFE};
	const char aStrUtf16be[] = {0xFE, 0xFF};

	if (strncmp((const char *)pucBufHead, aStrUtf8, 3) == 0) {
		/* utf-8 */
		return 0;
	} else if (strncmp((const char *)pucBufHead, aStrUtf16le, 2) == 0) {
		/* utf-16 little endian */
		return 0;
	} else if (strncmp((const char *)pucBufHead, aStrUtf16be, 2) == 0) {
		/* utf-16 big endian */
		return 0;
	} else {
		return 1;
	}
}

/* return get code byte num */
static int AsciiGetCodeFrmBuf(unsigned char *pucBufStart,
		unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	unsigned char *pucBuf = pucBufStart;
	unsigned char c = *pucBuf;

	if ((pucBuf <= pucBufEnd) && (c < (unsigned char)0x80)) {
		/* ASCII code */
		*pdwCode = (unsigned int)c;
		return 1;
	}

	if (((pucBuf + 1) <= pucBufEnd) && (c >= (unsigned char)0x80)) {
		/* GBK code, big endian */
		//*pdwCode = pucBuf[0] + (pucBuf[1]<<8);//´óÐ¡¶Ë????
		*pdwCode =  (pucBuf[0]<<8) + pucBuf[1];
		return 2;
	}
	
	if (pucBuf <= pucBufEnd) {
		*pdwCode = (unsigned int)c;
		return 1;
	} else {
		return 0;
	}
}

static T_Encoding_Opr g_tAsciiEncodingOpr = {
	.c_pEncodingName = "ascii",
	.iHeadLen  = 0,
	.isSupport = isAsciiCoding,
	.Get_Code  = AsciiGetCodeFrmBuf,
};

int Ascii_Encoding_Init(void)
{
	Add_Font_Opr_For_Encoding(&g_tAsciiEncodingOpr, Get_Font_Opr("ascii"));
	Add_Font_Opr_For_Encoding(&g_tAsciiEncodingOpr, Get_Font_Opr("gbk"));
	return Encoding_Opr_Regisiter(&g_tAsciiEncodingOpr);
}

