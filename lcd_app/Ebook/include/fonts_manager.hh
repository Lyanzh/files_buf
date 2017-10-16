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
	int (*Font_Init)(char *pcFileName, unsigned int dwFontSize);
	int (*Get_Bitmap)(unsigned int dwCode, PT_Font_Para ptFontPara);
	void (*Font_Exit)(void);
	struct Font_Operation *ptNextFont;
}T_Font_Opr, *PT_Font_Opr;

int Ascii_Opr_Init(void);
int Gbk_Opr_Init(void);
int Freetype_Opr_Init(void);

extern int Font_Opr_Regisiter(PT_Font_Opr ptFontOpr);
extern void Show_Font_Opr(void);
extern PT_Font_Opr Get_Font_Opr(char *pcName);
extern int Font_Opr_Init(void);

#endif

