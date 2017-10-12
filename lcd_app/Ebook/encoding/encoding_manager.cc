#include "encoding_manager.h"
#include <stdio.h>

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
		ptEncodingOpr->ptNextEncoding = NULL;
	}

	return 0;
}

void Show_Encoding_Opr(void)
{
	int i = 0;
	PT_Encoding_Opr ptEncodingOprTmp = g_ptEncodingOprHead;
	while (ptEncodingOprTmp) {
		printf("%d %s\n", i++, ptEncodingOprTmp->c_pEncodingName);
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

int Encoding_Init(void)
{
	return Ascii_Encoding_Init();
}

