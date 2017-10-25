#ifndef __ENCODING_MANAGER__
#define __ENCODING_MANAGER__

#include "fonts_manager.h"

typedef struct Encoding_Operation
{
	const char * c_pEncodingName;
	int iHeadLen;
	PT_Font_Opr ptFontOprSupportedHead;
	int (*isSupport)(unsigned char *pucBufHead);
	int (*Get_Code)(unsigned char *pucBufStart,
			unsigned char *pucBufEnd, unsigned int *pdwCode);
	struct Encoding_Operation *ptNextEncoding;
}T_Encoding_Opr, *PT_Encoding_Opr;

int Ascii_Encoding_Init(void);
int Utf8_Encoding_Init(void);
int Utf16le_Encoding_Init(void);
int Utf16be_Encoding_Init(void);


extern int Encoding_Opr_Regisiter(PT_Encoding_Opr ptEncodingOpr);
extern void Show_Encoding_Opr(void);
extern PT_Encoding_Opr Select_Encoding_Opr(unsigned char *pucFileBufHead);
extern int Add_Font_Opr_For_Encoding(PT_Encoding_Opr ptEncodingOpr,
		PT_Font_Opr ptFontOprSupported);
extern int Del_Font_Opr_from_Encoding(PT_Encoding_Opr ptEncodingOprForFile,
		PT_Font_Opr ptFontOpr);
extern int Encoding_Opr_Init(void);

#endif
