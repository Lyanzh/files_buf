#include "fonts_manager.h"
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
 
//#include "memwatch.h"

static int g_iGbkFd;
static char *g_pcGbkMem;
static struct stat g_tGbkStat;

static unsigned int g_dwFontSize;

static int Gbk_Get_Bitmap(unsigned int dwCode, PT_Font_Para ptFontPara)
{
	unsigned int dwArea = ((dwCode >> 8) & 0xFF) - 0xA1;
	unsigned int dwWhere = (dwCode & 0xFF) - 0xA1;

	/* 传入的需要绘制的起点 */
	int iPenX = ptFontPara->iCurOriginX;
	int iPenY = ptFontPara->iCurOriginY;

	/* 根据传入的需要绘制的起点，计算实际绘制的起点以及绘制范围的大小 */
	ptFontPara->iXLeft = iPenX;
	ptFontPara->iYTop  = iPenY - g_dwFontSize;
	ptFontPara->iXmax  = ptFontPara->iXLeft + g_dwFontSize;
	ptFontPara->iYmax  = iPenY;

	ptFontPara->iPitch = 2;
	ptFontPara->iBpp = 1;

	ptFontPara->pucBuffer = (unsigned char *)(g_pcGbkMem + (dwArea * 94 + dwWhere) * 32);
	
	/* increment pen position */
	ptFontPara->iNextOriginX = iPenX + g_dwFontSize;
	ptFontPara->iNextOriginY = iPenY;

	return 0;
}

static int Gbk_Init(char *pcFileName, unsigned int dwFontSize)
{
	g_iGbkFd = open(pcFileName, O_RDONLY);
	if(g_iGbkFd < 0)
	{
		printf("Error:cannot open %s.\n", pcFileName);
		return -1;
	}

	//get HZK16 file information
	if(fstat(g_iGbkFd, &g_tGbkStat))
	{
		printf("Error:cannot get HZK16 stat.\n");
		return -1;
	}

	//map the device to memory
	g_pcGbkMem = (char *)mmap(0, g_tGbkStat.st_size, PROT_READ, MAP_SHARED, g_iGbkFd, 0);
	if((int)g_pcGbkMem == -1)
	{
		printf("Error:fail to map HZK16 to memory.\n");
		return -1;
	}

	g_dwFontSize = dwFontSize;

	return 0;
}

static void Gbk_Exit(void)
{
	munmap(g_pcGbkMem, g_tGbkStat.st_size);
	close(g_iGbkFd);
}

static T_Font_Opr g_tGbkOpr = {
	.c_pFontName = "gbk",
	.Font_Init   = Gbk_Init,
	.Get_Bitmap  = Gbk_Get_Bitmap,
	.Font_Exit   = Gbk_Exit,
};

int Gbk_Opr_Init(void)
{
	return Font_Opr_Regisiter(&g_tGbkOpr);
}

