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
	PT_Font_Opr ptFontOprTmp;
	if (!ptEncodingOpr->ptFontOprSupportedHead) {
		ptEncodingOpr->ptFontOprSupportedHead = ptFontOprSupported;
	} else {
		ptFontOprTmp = ptEncodingOpr->ptFontOprSupportedHead;
		while (ptFontOprTmp->ptNextFont) {
			ptFontOprTmp = ptFontOprTmp->ptNextFont;
		}
		ptFontOprTmp->ptNextFont = ptFontOprSupported;
	}
	ptFontOprSupported->ptNextFont = NULL;
}

int Encoding_Opr_Init(void)
{
	int iError;
	iError =  Ascii_Encoding_Init();
	if (iError) {
		printf("Error:ASCII encoding init fail.\n");
		return -1;
	}

#if 1
	iError =  Utf8_Encoding_Init();
	if (iError) {
		printf("Error:UTF-8 encoding init fail.\n");
		return -1;
	}
	
	iError =  Utf16le_Encoding_Init();
	if (iError) {
		printf("Error:UTF-16LE encoding init fail.\n");
		return -1;
	}

	iError =  Utf16be_Encoding_Init();
	if (iError) {
		printf("Error:UTF-16BE encoding init fail.\n");
		return -1;
	}
#endif
	
	return 0;
}

