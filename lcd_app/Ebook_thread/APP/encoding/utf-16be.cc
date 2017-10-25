#include "encoding_manager.h"
#include <string.h>

#include "memwatch.h"

/* return: 1->yes, 0->no */
static int isUtf16beCoding(unsigned char *pucBufHead)
{
	const char aStrUtf16be[] = {0xFE, 0xFF};

	if (strncmp((const char *)pucBufHead, aStrUtf16be, 2) == 0) {
		/* utf-16 big endian */
		return 1;
	} else {
		return 0;
	}
}

/* return get code byte num */
static int Utf16be_Get_Code(unsigned char *pucBufStart,
		unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	if ((pucBufStart + 1) <= pucBufEnd) {
		*pdwCode = (pucBufStart[0]<<8) + pucBufStart[1];/* big endian */
		return 2;
	} else {
		/* file end */
		return 0;
	}
}

static T_Encoding_Opr g_tUtf16beEncodingOpr = {
	.c_pEncodingName = "utf-16be",
	.iHeadLen  = 2,
	.isSupport = isUtf16beCoding,
	.Get_Code  = Utf16be_Get_Code,
};

int Utf16be_Encoding_Init(void)
{
	Add_Font_Opr_For_Encoding(&g_tUtf16beEncodingOpr, Get_Font_Opr("freetype"));
	return Encoding_Opr_Regisiter(&g_tUtf16beEncodingOpr);
}

