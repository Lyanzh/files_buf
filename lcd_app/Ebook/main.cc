#include "draw.h"
#include "encoding_manager.h"
#include "fonts_manager.h"
#include "disp_manager.h"

#include "memwatch.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	char cOpr;
	unsigned int pdwCode;
	T_Font_Para tFontPara;

	Disp_Opr_Init();
	Font_Opr_Init();
	Encoding_Opr_Init();

	Show_Disp_Opr();
	Show_Font_Opr();
	Show_Encoding_Opr();

	Select_And_Init_Display("s3c2440-lcd");
	g_ptDispOpr->Clean_Screen();

	Open_Text_File("text_ansi.txt");
	printf("open text file success.\n");
	g_ptEncodingOprForFile->Get_Code(g_pucLcdFirstPosAtFile, g_pucFileMemEnd, &pdwCode);
	printf("0x%x\n", pdwCode);

	printf("%s\n", g_ptEncodingOprForFile->c_pEncodingName);
	printf("%d\n", g_ptEncodingOprForFile->iHeadLen);
	printf("%s\n", g_ptEncodingOprForFile->ptFontOprSupportedHead->c_pFontName);

	g_ptEncodingOprForFile->ptFontOprSupportedHead->Font_Init("simsun.ttc", 24);

	tFontPara.iCurOriginX = 0;
	tFontPara.iCurOriginY = 24;
	g_ptEncodingOprForFile->ptFontOprSupportedHead->Get_Bitmap(pdwCode, &tFontPara);
	printf("iXLeft = %d\n", tFontPara.iXLeft);
	printf("iYTop = %d\n", tFontPara.iYTop);
	printf("iXmax = %d\n", tFontPara.iXmax);
	printf("iYmax = %d\n", tFontPara.iYmax);
	Draw_Bitmap(&tFontPara);

	while (1) {
		printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: ");
		do {
			cOpr = getchar();
		} while (cOpr != 'n' && cOpr != 'u' &&cOpr != 'q');

		if (cOpr == 'n') {
			printf("show next page.\n");
		} else if (cOpr == 'u') {
			printf("show pre page.\n");
		} else if (cOpr == 'q') {
			printf("quit.\n");
			return 0;
		}
	}

	return 0;
}
