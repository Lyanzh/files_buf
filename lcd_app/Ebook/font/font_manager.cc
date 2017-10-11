#include "font_manager.h"
#include <stdio.h>

static PT_Font_Opr g_ptFontOprHead;

int Font_Opr_Regisiter(PT_Font_Opr ptFontOpr)
{
	PT_Font_Opr ptFontOprTmp;
	
	if (!g_ptFontOprHead) {
		g_ptFontOprHead = ptFontOpr;
	} else {
		ptFontOprTmp = g_ptFontOprHead;
		while (ptFontOprTmp->ptNextFont) {
			ptFontOprTmp = ptFontOprTmp->ptNextFont;
		}
		ptFontOprTmp->ptNextFont = ptFontOpr;
		ptFontOpr->ptNextFont = NULL;
	}

	return 0;
}

int Font_Init(void)
{
	return Freetype_Opr_Init();
}
