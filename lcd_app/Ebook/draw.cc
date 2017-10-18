#include "draw.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "memwatch.h"

PT_Encoding_Opr g_ptEncodingOprForFile;
PT_DispDev g_ptDispOpr;

unsigned char *g_pucFileMemStart;
unsigned char *g_pucFileMemEnd;

/*static*/ unsigned char *g_pucLcdFirstPosAtFile;
static unsigned char *g_pucLcdNextPosAtFile;

unsigned int g_dwFontSize;

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

static void Del_Font_Opr_from_Encoding(PT_Encoding_Opr ptEncodingOprForFile,
		PT_Font_Opr ptFontOpr)
{
	PT_Font_Opr ptFontOprTmp;
	
	if (!ptEncodingOprForFile->ptFontOprSupportedHead) {
		return;
	}

	ptFontOprTmp = ptEncodingOprForFile->ptFontOprSupportedHead;
	while (ptFontOprTmp) {
		if (strcmp(ptFontOprTmp->c_pFontName, ptFontOpr->c_pFontName) == 0) {
			/* delete */
			
		} else {
			ptFontOprTmp = ptFontOprTmp->ptNextFont;
		}
	}
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
			Del_Font_Opr_from_Encoding();
		}
		ptFontOpr = ptFontOprTmp;
	}
	return iRet;
}

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

static int Inc_LcdY(int iY)
{
	if (iY + g_dwFontSize < g_ptDispOpr->tDevAttr.dwYres) {
		return (iY + g_dwFontSize);
	} else {
		/* page full */
		return 0;
	}
}

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
		/* pos to next line */
		iLcdY = Inc_LcdY(ptFontPara->iCurOriginY);
		if (0 == iLcdY) {
			/* page full */
			return -1;
		} else {
			/* page not full */
		}
	}
}

int Show_One_Font(PT_Font_Para ptFontPara)
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

int Show_One_Page(unsigned char *pucTextFileMemCurPos)
{
	int iLen;
	unsigned int dwCode;
	unsigned char *pucBufStart;

	PT_Font_Opr ptFontOprTmp;
	T_Font_Para tFontPara;

	int iError;

	unsigned char bHasNotClrScreen = 1;
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
				continue;
			}
		} else if (dwCode == '\r') {
			continue;
		} else if (dwCode == '\t') {
			dwCode = ' ';
		}

		ptFontOprTmp = g_ptEncodingOprForFile->ptFontOprSupportedHead;
		while (ptFontOprTmp) {
			iError = ptFontOprTmp->Get_Bitmap(dwCode, &tFontPara);
			if (0 == iError) {
				if (Relocate_Font_Pos(&tFontPara)) {
					/* cannot show this code on this page */
					return 0;
				}

				if (bHasNotClrScreen) {
					/* clear screen first */
					g_ptDispOpr->Clean_Screen();
					bHasNotClrScreen = 0;
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

void Draw_Bitmap(PT_Font_Para ptFontPara)
{
	//int i, j, p, q;

	//int iBitmapWidth = ptFontPara->iXmax - ptFontPara->iXLeft;

#if 0
	for (i = ptFontPara->iCurOriginX, p = 0; i < ptFontPara->iXmax; i++, p++)
	{
		for (j = ptFontPara->iCurOriginY, q = 0; j < ptFontPara->iYmax; j++, q++)
		{
			if (i < 0 || j < 0 || i >= 480 || j >= 272)
				continue;

			g_ptDispOpr->Put_Pixel(i, j, ptFontPara->pucBuffer[q * iBitmapWidth + p]);
		}
	}
#endif
	int i, b;
	unsigned char byte;

	int x = ptFontPara->iCurOriginX;
	int y = ptFontPara->iCurOriginY;

	for(i = 0; i < 16; i++)
	{
		byte = ptFontPara->pucBuffer[i];
		for(b = 7; b >= 0; b--)
		{
			if(byte & (1<<b))
			{
				g_ptDispOpr->Put_Pixel(x+7-b, y+i, 0xFFFF);//on
			}
			else
			{
				g_ptDispOpr->Put_Pixel(x+7-b, y+i, 0);//off
			}
		}
	}
}

