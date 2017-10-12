#ifndef __FONT_MANAGER__
#define __FONT_MANAGER__

typedef struct FontParameter
{
	int iXLeft;
	int iYTop;
	int iXmax;
	int iYmax;
	int iCurOriginX;
	int iCurOriginY;
	int iNextOriginX;
	int iNextOriginY;
	unsigned char *pucBuffer;
}T_Font_Para, *PT_Font_Para;

typedef struct Font_Operation
{
	const char * c_pFontName;
	int (*Font_Init)(char *pcFileName, unsigned int font_size);
	int (*Get_Bitmap)(unsigned int dwCode, PT_Font_Para ptFontPara);
	int (*Font_Exit)(void);
	struct Font_Operation *ptNextFont;
}T_Font_Opr, *PT_Font_Opr;

extern int Font_Opr_Regisiter(PT_Font_Opr ptFontOpr);
extern int Font_Init(void);

#endif
