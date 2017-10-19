#include "draw.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "memwatch.h"

PT_Encoding_Opr g_ptEncodingOprForFile;
PT_DispDev g_ptDispOpr;

typedef struct PageDesc {
	int iPage;
	unsigned char *pucLcdFirstPosAtFile;
	unsigned char *pucLcdNextPageFirstPosAtFile;
	struct PageDesc *ptPrePage;
	struct PageDesc *ptNextPage;
} T_PageDesc, *PT_PageDesc;

static PT_PageDesc g_ptPagesHead = NULL;
static PT_PageDesc g_ptCurPage = NULL;

static unsigned char *g_pucFileMemStart;
static unsigned char *g_pucFileMemEnd;

static unsigned char *g_pucLcdFirstPosAtFile;
static unsigned char *g_pucLcdNextPosAtFile;

static unsigned int g_dwFontSize;

int Select_And_Init_Display(char *pcName)
{
	int iError;
	g_ptDispOpr = Get_Disp_Opr(pcName);
	if (!g_ptDispOpr) {
		printf("Error:can not get display operation for %s.", pcName);
		return -1;
	}

	iError = g_ptDispOpr->Dev_Init();
	return 0;
}

int Open_Text_File(char *pcPathName)
{
	int iFd;
	struct stat tFileStat;
	int iError;

	iFd = open(pcPathName, O_RDONLY);
	if (iFd < 0) {
		printf("Error:can't open file %s.\n", pcPathName);
		return -1;
	}

	iError = fstat(iFd, &tFileStat);
	if (iError) {
		printf("Error:get file %s information error.\n", pcPathName);
		close(iFd);
		return -1;
	}

	g_pucFileMemStart = (unsigned char *)mmap(NULL, tFileStat.st_size,
			PROT_READ, MAP_SHARED, iFd, 0);
	if (g_pucFileMemStart == (unsigned char *)-1) {
		printf("Error:map file error.\n");
		close(iFd);
		return -1;
	}

	g_pucFileMemEnd = g_pucFileMemStart + tFileStat.st_size - 1;

	g_ptEncodingOprForFile = Select_Encoding_Opr(g_pucFileMemStart);

	if (g_ptEncodingOprForFile) {
		g_pucLcdFirstPosAtFile = g_pucFileMemStart + g_ptEncodingOprForFile->iHeadLen;
	} else {
		printf("Error:get encoding operation for file error.\n");
		close(iFd);
		return -1;
	}

	close(iFd);
	return 0;
}

static int Del_Font_Opr_from_Encoding(PT_Encoding_Opr ptEncodingOprForFile,
		PT_Font_Opr ptFontOpr)
{
	PT_Font_Opr ptFontOprPre;
	PT_Font_Opr ptFontOprCur;
	
	if (!ptEncodingOprForFile->ptFontOprSupportedHead) {
		return -1;
	}
	
	if (strcmp(ptEncodingOprForFile->ptFontOprSupportedHead->c_pFontName, ptFontOpr->c_pFontName) == 0) {
		/* delete head */
		ptEncodingOprForFile->ptFontOprSupportedHead = ptEncodingOprForFile->ptFontOprSupportedHead->ptNextFont;
		return 0;
	} else {
		ptFontOprPre = ptEncodingOprForFile->ptFontOprSupportedHead;
		ptFontOprCur = ptFontOprPre->ptNextFont;
		while (ptFontOprCur) {
			if (strcmp(ptFontOprCur->c_pFontName, ptFontOpr->c_pFontName) == 0) {
				/* delete */
				ptFontOprPre->ptNextFont = ptFontOprCur->ptNextFont;
				return 0;
			} else {
				ptFontOprPre = ptFontOprCur;
				ptFontOprCur = ptFontOprCur->ptNextFont;
			}
		}
	}
	return -1;
}

int Set_Text_Detail(char *pcHZKFile, char *pcFreetypeFile, unsigned int dwFontSize)
{
	int iError;
	int iRet;
	PT_Font_Opr ptFontOpr;
	PT_Font_Opr ptFontOprTmp;

	g_dwFontSize = dwFontSize;

	ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;
	while (ptFontOpr) {
		if (strcmp(ptFontOpr->c_pFontName, "ascii") == 0) {
			iError = ptFontOpr->Font_Init(NULL, dwFontSize);
		} else if (strcmp(ptFontOpr->c_pFontName, "gbk") == 0) {
			iError = ptFontOpr->Font_Init(pcHZKFile, dwFontSize);
		} else {
			iError = ptFontOpr->Font_Init(pcFreetypeFile, dwFontSize);
		}

		ptFontOprTmp = ptFontOpr->ptNextFont;

		if (iError == 0) {
			iRet = 0;
		} else {
			Del_Font_Opr_from_Encoding(g_ptEncodingOprForFile, ptFontOpr);
		}
		ptFontOpr = ptFontOprTmp;
	}
	return iRet;
}

static int Inc_LcdY(int iY)
{
	if (iY + g_dwFontSize < g_ptDispOpr->tDevAttr.dwYres) {
		return (iY + g_dwFontSize);
	} else {
		/* page full */
		return 0;
	}
}

/* 处理有可能的行满或页满的情况 */
static int Relocate_Font_Pos(PT_Font_Para ptFontPara)
{
	int iLcdY;
	int iDeltaX;
	int iDeltaY;

	if (ptFontPara->iYmax > g_ptDispOpr->tDevAttr.dwYres) {
		/* page full */
		return -1;
	}

	if (ptFontPara->iXmax > g_ptDispOpr->tDevAttr.dwXres) {
		/* try to relocate to next line */
		iLcdY = Inc_LcdY(ptFontPara->iCurOriginY);
		if (0 == iLcdY) {
			/* page full */
			return -1;
		} else {
			/* page not full, relocate to next line */
			iDeltaX = 0 - ptFontPara->iCurOriginX;
			iDeltaY = iLcdY - ptFontPara->iCurOriginY;

			ptFontPara->iCurOriginX += iDeltaX;
			ptFontPara->iCurOriginY += iDeltaY;
			
			ptFontPara->iXLeft += iDeltaX;
			ptFontPara->iYTop += iDeltaY;

			ptFontPara->iXmax += iDeltaX;
			ptFontPara->iYmax += iDeltaY;

			ptFontPara->iNextOriginX += iDeltaX;
			ptFontPara->iNextOriginY += iDeltaY;

			return 0;
		}
	}

	return 0;
}

static int Show_One_Font(PT_Font_Para ptFontPara)
{
	int x;
	int y;
	int iIndex;
	int bit;
	unsigned char ucByte;

	if (ptFontPara->iBpp == 1) {
		for (y = ptFontPara->iYTop; y < ptFontPara->iYmax; y++) {
			iIndex = (y - ptFontPara->iYTop) * ptFontPara->iPitch;
			for (x = ptFontPara->iXLeft, bit = 7; x < ptFontPara->iXmax; x++) {
				if (bit == 7) {
					/* get the data */
					ucByte = ptFontPara->pucBuffer[iIndex];
				}

				if (ucByte & (1<<bit)) {
					g_ptDispOpr->Put_Pixel(x, y, 0xFFFFFF);
				} else {
					g_ptDispOpr->Put_Pixel(x, y, 0);
				}

				bit--;
				if (bit == -1) {
					bit = 7;
					iIndex++;//next byte
				}
			}
		}
	} else if (ptFontPara->iBpp == 8) {
		iIndex = 0;
		for (y = ptFontPara->iYTop; y < ptFontPara->iYmax; y++) {
			for (x = ptFontPara->iXLeft; x < ptFontPara->iXmax; x++) {
				if (ptFontPara->pucBuffer[iIndex++])
					g_ptDispOpr->Put_Pixel(x, y, 0xFFFFFF);
			}
		}
	} else {
		printf("Error:Show font cannot support %d bpp.\n", ptFontPara->iBpp);
		return -1;
	}
	return 0;
}

static int Show_One_Page(unsigned char *pucTextFileMemCurPos)
{
	int iLen;
	unsigned int dwCode;
	unsigned char *pucBufStart;

	PT_Font_Opr ptFontOprTmp;
	T_Font_Para tFontPara;

	int iError;

	unsigned char bHasClrScreen = 0;
	unsigned char bHasGetCode = 0;

	tFontPara.iCurOriginX = 0;
	tFontPara.iCurOriginY = g_dwFontSize;
	
	pucBufStart = pucTextFileMemCurPos;
	
	while (1) {
		iLen = g_ptEncodingOprForFile->Get_Code(pucBufStart, g_pucFileMemEnd, &dwCode);
		if (iLen == 0) {
			/* file end */
			if (!bHasGetCode) {
				return -1;
			} else {
				return 0;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		if (dwCode == '\n') {
			g_pucLcdNextPosAtFile = pucBufStart;

			/* next line */
			tFontPara.iCurOriginX = 0;
			tFontPara.iCurOriginY = Inc_LcdY(tFontPara.iCurOriginY);
			if (0 == tFontPara.iCurOriginY) {
				/* Current page has over */
				return 0;
			} else {
				/* show next code */
				continue;
			}
		} else if (dwCode == '\r') {
			/* show next code */
			continue;
		} else if (dwCode == '\t') {
			dwCode = ' ';
		}

		ptFontOprTmp = g_ptEncodingOprForFile->ptFontOprSupportedHead;
		while (ptFontOprTmp) {
			printf("Get_Bitmap.\n");
			iError = ptFontOprTmp->Get_Bitmap(dwCode, &tFontPara);
			printf("Get_Bitmap over.\n");
			if (0 == iError) {
				if (Relocate_Font_Pos(&tFontPara)) {
					/* no more space to show this code on this page */
					return 0;
				}

				if (!bHasClrScreen) {
					/* clear screen first */
					g_ptDispOpr->Clean_Screen();
					bHasClrScreen = 1;
				}

				if (Show_One_Font(&tFontPara)) {
					return -1;
				}

				tFontPara.iCurOriginX = tFontPara.iNextOriginX;
				tFontPara.iCurOriginY = tFontPara.iNextOriginY;
				g_pucLcdNextPosAtFile = pucBufStart;

				/* show current code over, get next code to show */
				break;
			}
			ptFontOprTmp = ptFontOprTmp->ptNextFont;
		}
	}

	return 0;
}

static void Record_Page(PT_PageDesc ptPageNew)
{
	PT_PageDesc ptPageTmp;
	
	if (!g_ptPagesHead) {
		g_ptPagesHead = ptPageNew;
	} else {
		ptPageTmp = g_ptPagesHead;
		while (ptPageTmp->ptNextPage) {
			ptPageTmp = ptPageTmp->ptNextPage;
		}

		ptPageTmp->ptNextPage = ptPageNew;
		ptPageNew->ptPrePage = ptPageTmp;
	}
}

int Show_Next_Page(void)
{
	int iError;
	PT_PageDesc ptPage;
	unsigned char *pucTextFileMemCurPos;

	if (g_ptCurPage) {
		pucTextFileMemCurPos = g_ptCurPage->pucLcdNextPageFirstPosAtFile;
	} else {
		pucTextFileMemCurPos = g_pucLcdFirstPosAtFile;
	}

	iError = Show_One_Page(pucTextFileMemCurPos);
	if (0 == iError) {
		if (g_ptCurPage && g_ptCurPage->ptNextPage) {
			g_ptCurPage = g_ptCurPage->ptNextPage;
		}

		ptPage = (PT_PageDesc)malloc(sizeof(T_PageDesc));
		if (ptPage) {
			ptPage->pucLcdFirstPosAtFile = pucTextFileMemCurPos;
			ptPage->pucLcdNextPageFirstPosAtFile = g_pucLcdNextPosAtFile;
			ptPage->ptPrePage = NULL;
			ptPage->ptNextPage = NULL;
			
			g_ptCurPage = ptPage;
			Record_Page(ptPage);
			return 0;
		} else {
			return -1;
		}
	}

	return iError;
}

int Show_Pre_Page(void)
{
	int iError;

	if (!g_ptCurPage || !g_ptCurPage->ptPrePage)
		return -1;

	iError = Show_One_Page(g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	if (0 == iError) {
		g_ptCurPage = g_ptCurPage->ptNextPage;
	}
	return iError;
}

