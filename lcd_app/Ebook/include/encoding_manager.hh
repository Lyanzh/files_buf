#ifndef __ENCODING_MANAGER__
#define __ENCODING_MANAGER__

#include "fonts_manager.h"

typedef struct Encoding_Operation
{
	const char * c_pEncodingName;
	int iHeadLen;
	PT_Font_Opr ptFontOprSupportedHead;
	int (*isSupport)(unsigned char *pucBufHead);
	int (*Get_Code)();
	struct Encoding_Operation *ptNextEncoding;
}T_Encoding_Opr, *PT_Encoding_Opr;

extern int Encoding_Opr_Regisiter(PT_Encoding_Opr ptEncodingOpr);
extern int Encoding_Init(void);

#endif
