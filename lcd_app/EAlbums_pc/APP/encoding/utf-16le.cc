#include "encoding_manager.h"
#include <string.h>
#include <stdio.h>

#include "memwatch.h"

/* return: 1->yes, 0->no */
static int isUtf16leCoding(unsigned char *pucBufHead)
{
	const char aStrUtf16le[] = {0xFF, 0xFE};

	if (strncmp((const char *)pucBufHead, aStrUtf16le, 2) == 0) {
		/* utf-16 little endian */
		return 1;
	} else {
		return 0;
	}
}

/* return get code byte num */
static int Utf16le_Get_Code(unsigned char *pucBufStart,
		unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	if ((pucBufStart + 1) <= pucBufEnd) {
		*pdwCode = pucBufStart[0] + (pucBufStart[1]<<8);/* little endian */
		return 2;
	} else {
		/* file end */
		return 0;
	}
}

static T_Encoding_Opr g_tUtf16leEncodingOpr = {
	.c_pEncodingName = "utf-16le",
	.iHeadLen  = 2,
	.isSupport = isUtf16leCoding,
	.Get_Code  = Utf16le_Get_Code,
};

int Utf16le_Encoding_Init(void)
{
	Add_Font_Opr_For_Encoding(&g_tUtf16leEncodingOpr, Get_Font_Opr("freetype"));
	return Encoding_Opr_Regisiter(&g_tUtf16leEncodingOpr);
}

