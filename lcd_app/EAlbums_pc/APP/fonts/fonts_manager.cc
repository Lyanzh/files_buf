#include "fonts_manager.h"
#include <stdio.h>
#include <string.h>

//#include "memwatch.h"

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
	}
	ptFontOpr->ptNextFont = NULL;

	return 0;
}

void Show_Font_Opr(void)
{
	int i = 0;
	PT_Font_Opr ptFontOprTmp = g_ptFontOprHead;
	while (ptFontOprTmp) {
		printf("%d: %s\n", i++, ptFontOprTmp->c_pFontName);
		ptFontOprTmp = ptFontOprTmp->ptNextFont;
	}
}

PT_Font_Opr Get_Font_Opr(char *pcName)
{
	PT_Font_Opr ptFontOprTmp = g_ptFontOprHead;
	while (ptFontOprTmp) {
		if (strcmp(ptFontOprTmp->c_pFontName, pcName) == 0) {
			//printf("get font %s.\n", pcName);
			return ptFontOprTmp;
		} else {
			ptFontOprTmp = ptFontOprTmp->ptNextFont;
		}
	}
	printf("Error:can't get font %s.\n", pcName);
	return NULL;
}

int Font_Opr_Init(void)
{
	int iError;
	
	iError = Ascii_Opr_Init();
	if (iError) {
		printf("Error:ASCII init error.\n");
		return -1;
	}

	iError = Gbk_Opr_Init();
	if (iError) {
		printf("Error:GBK init error.\n");
		return -1;
	}
	
	iError = Freetype_Opr_Init();
	if (iError) {
		printf("Error:Freetype init error.\n");
		return -1;
	}
	
	return 0;
}

