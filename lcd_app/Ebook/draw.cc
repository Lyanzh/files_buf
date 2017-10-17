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

unsigned char *g_pucLcdFirstPosAtFile;

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

