#include "encoding_manager.h"
#include "fonts_manager.h"
#include "disp_manager.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

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
		return -1;
	}

	g_pucFileMemStart = (unsigned char *)mmap(NULL, tFileStat.st_size,
			PROT_READ, MAP_SHARED, iFd, 0);
	if (g_pucFileMemStart == (unsigned char *)-1) {
		printf("Error:map file error.\n");
		return -1;
	}

	g_pucFileMemEnd = g_pucFileMemStart + tFileStat.st_size;

	g_ptEncodingOprForFile = Select_Encoding_Opr(g_pucFileMemStart);

	if (g_ptEncodingOprForFile) {
		g_pucLcdFirstPosAtFile = g_pucFileMemStart + g_ptEncodingOprForFile.iHeadLen;
	} else {
		printf("Error:get encoding operation for file error.\n");
		return -1;
	}

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

	iError = g_ptDispOpr->Dev_Init;
}

