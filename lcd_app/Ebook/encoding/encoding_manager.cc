#include "encoding_manager.h"
#include <stdio.h>

#include "memwatch.h"

static PT_Encoding_Opr g_ptEncodingOprHead;

int Encoding_Opr_Regisiter(PT_Encoding_Opr ptEncodingOpr)
{
	PT_Encoding_Opr ptEncodingOprTmp;
	
	if (!g_ptEncodingOprHead) {
		g_ptEncodingOprHead = ptEncodingOpr;
	} else {
		ptEncodingOprTmp = g_ptEncodingOprHead;
		while (ptEncodingOprTmp->ptNextEncoding) {
			ptEncodingOprTmp = ptEncodingOprTmp->ptNextEncoding;
		}
		ptEncodingOprTmp->ptNextEncoding = ptEncodingOpr;
	}
	ptEncodingOpr->ptNextEncoding = NULL;

	return 0;
}

void Show_Encoding_Opr(void)
{
	int i = 0;
	PT_Encoding_Opr ptEncodingOprTmp = g_ptEncodingOprHead;
	while (ptEncodingOprTmp) {
		printf("%d: %s\n", i, ptEncodingOprTmp->c_pEncodingName);
		printf("%d: %s\n", i++, ptEncodingOprTmp->ptFontOprSupportedHead->c_pFontName);
		ptEncodingOprTmp = ptEncodingOprTmp->ptNextEncoding;
	}
}

PT_Encoding_Opr Select_Encoding_Opr(unsigned char *pucFileBufHead)
{
	PT_Encoding_Opr ptEncodingOprTmp = g_ptEncodingOprHead;
	while (ptEncodingOprTmp) {
		if (ptEncodingOprTmp->isSupport(pucFileBufHead))
			return ptEncodingOprTmp;
		else
			ptEncodingOprTmp = ptEncodingOprTmp->ptNextEncoding;
	}
	return NULL;
}

void Add_Font_Opr_For_Encoding(PT_Encoding_Opr ptEncodingOpr,
		PT_Font_Opr ptFontOprSupported)
{
	PT_Font_Opr ptFontOprTmp = ptEncodingOpr->ptFontOprSupportedHead;
	if (!ptFontOprTmp) {
		ptFontOprTmp = ptFontOprSupported;
	} else {
		while (ptFontOprTmp->ptNextFont) {
			ptFontOprTmp = ptFontOprTmp->ptNextFont;
		}
		ptFontOprTmp->ptNextFont = ptFontOprSupported;
	}
	printf("Add_Font_Opr_For_Encoding %s.\n", ptFontOprTmp->c_pFontName);
	ptFontOprSupported->ptNextFont = NULL;
}

int Encoding_Opr_Init(void)
{
	return Ascii_Encoding_Init();
}

