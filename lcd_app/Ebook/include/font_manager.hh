#ifndef __FONT_MANAGER__
#define __FONT_MANAGER__

typedef struct Font_Operation
{
	const char * c_pFontName;
	int (*Font_Init)(char *font_filename, unsigned int font_size);
	int (*Font_Exit)(void);
	struct Font_Operation *ptNextFont;
}T_Font_Opr, *PT_Font_Opr;

#endif

