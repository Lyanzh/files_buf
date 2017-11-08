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

typedef struct PageDesc {
	int iPage;
	unsigned char *pucPageFirstPosAtFileMem;
	unsigned char *pucPageEndPosAtFileMem;
	struct PageDesc *ptPrePage;
	struct PageDesc *ptNextPage;
} T_PageDesc, *PT_PageDesc;

static PT_PageDesc g_ptPagesHead = NULL;
static PT_PageDesc g_ptCurPage = NULL;

static unsigned char *g_pucFileMemStart;
static unsigned char *g_pucFileMemEnd;

static unsigned char *g_pucTextFirstPosAtFileMem;
static unsigned char *g_pucCurPageTextEndPosAtFileMem;

static unsigned int g_dwFontSize;

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
	printf("FileMemStart = %x, FileMemEnd = %x.\n", g_pucFileMemStart, g_pucFileMemEnd);

	g_ptEncodingOprForFile = Select_Encoding_Opr(g_pucFileMemStart);

	if (g_ptEncodingOprForFile) {
		g_pucTextFirstPosAtFileMem = g_pucFileMemStart + g_ptEncodingOprForFile->iHeadLen;
	} else {
		printf("Error:get encoding operation for file error.\n");
		close(iFd);
		return -1;
	}

	close(iFd);
	return 0;
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
	if (iY + g_dwFontSize < g_ptDispOprSelected->tDevAttr.dwYres) {
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

	if (ptFontPara->iYmax > g_ptDispOprSelected->tDevAttr.dwYres) {
		/* page full */
		return -1;
	}

	if (ptFontPara->iXmax > g_ptDispOprSelected->tDevAttr.dwXres) {
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
					g_ptDispOprSelected->Put_Pixel(x, y, 0xFFFFFF);
				} else {
					g_ptDispOprSelected->Put_Pixel(x, y, 0);
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
					g_ptDispOprSelected->Put_Pixel(x, y, 0xFFFFFF);
			}
		}
	} else {
		printf("Error:Show font cannot support %d bpp.\n", ptFontPara->iBpp);
		return -1;
	}
	return 0;
}

static int Show_One_Page(unsigned char *pucPageFirstPosAtFileMem)
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
	
	pucBufStart = pucPageFirstPosAtFileMem;
	
	while (1) {
		iLen = g_ptEncodingOprForFile->Get_Code(pucBufStart, g_pucFileMemEnd, &dwCode);
		if (iLen == 0) {
			/* file end */
			if (bHasGetCode) {
				return 0;
			} else {
				return -1;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		if (dwCode == '\n') {
			g_pucCurPageTextEndPosAtFileMem = pucBufStart - 1;

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
			iError = ptFontOprTmp->Get_Bitmap(dwCode, &tFontPara);
			if (0 == iError) {
				if (Relocate_Font_Pos(&tFontPara)) {
					/* no more space to show this code on this page */
					return 0;
				}

				if (!bHasClrScreen) {
					/* clear screen first */
					g_ptDispOprSelected->Clean_Screen();
					bHasClrScreen = 1;
				}

				if (Show_One_Font(&tFontPara)) {
					return -1;
				}

				tFontPara.iCurOriginX = tFontPara.iNextOriginX;
				tFontPara.iCurOriginY = tFontPara.iNextOriginY;
				g_pucCurPageTextEndPosAtFileMem = pucBufStart - 1;

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
	static int i = 0;

	ptPageNew->iPage = i++;
	
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
	unsigned char *pucNextPageFirstPosAtFileMem;

	if (g_ptCurPage) {
		pucNextPageFirstPosAtFileMem = g_ptCurPage->pucPageEndPosAtFileMem + 1;
	} else {
		pucNextPageFirstPosAtFileMem = g_pucTextFirstPosAtFileMem;
	}

	iError = Show_One_Page(pucNextPageFirstPosAtFileMem);
	if (0 == iError) {
		if (g_ptCurPage && g_ptCurPage->ptNextPage) {
			g_ptCurPage = g_ptCurPage->ptNextPage;
		} else {
			ptPage = (PT_PageDesc)malloc(sizeof(T_PageDesc));
			if (ptPage) {
				ptPage->pucPageFirstPosAtFileMem = pucNextPageFirstPosAtFileMem;
				ptPage->pucPageEndPosAtFileMem = g_pucCurPageTextEndPosAtFileMem;
				ptPage->ptPrePage = NULL;
				ptPage->ptNextPage = NULL;
				
				Record_Page(ptPage);
				g_ptCurPage = ptPage;
			} else {
				return -1;
			}
		}
		return 0;
	} else {
		return -1;
	}
}

int Show_Pre_Page(void)
{
	int iError;

	if (!g_ptCurPage || !g_ptCurPage->ptPrePage)
		return -1;

	iError = Show_One_Page(g_ptCurPage->ptPrePage->pucPageFirstPosAtFileMem);
	if (0 == iError) {
		g_ptCurPage = g_ptCurPage->ptPrePage;
	}
	return iError;
}

void Show_Pages(void)
{
	PT_PageDesc ptPagesTmp = g_ptPagesHead;
	printf("Show pages:\n");
	while (ptPagesTmp) {
		printf("%d\n", ptPagesTmp->iPage);
		ptPagesTmp = ptPagesTmp->ptNextPage;
	}
	printf("Show pages end\n");
}

