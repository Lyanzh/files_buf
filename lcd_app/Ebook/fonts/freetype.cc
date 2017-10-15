#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>

#include "fonts_manager.h"

#include "memwatch.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library g_tLibrary;
FT_Face    g_tFace;
	
int Freetype_Get_Bitmap(unsigned int dwCode, PT_Font_Para ptFontPara)
{
	int error;

	FT_GlyphSlot tSlot;
	FT_Matrix	 tMatrix;	/* transformation matrix */
	FT_Vector	 tPen;		/* untransformed origin  */
	
	double dAngle;

	/* 传入的需要绘制的起点 */
	int iPenX = ptFontPara->iCurOriginX;
	int iPenY = ptFontPara->iCurOriginY;
	
	tSlot = g_tFace->glyph;

	dAngle = (0 / 360) * 3.14159 * 2;	   /* use 0 degrees */

	/* set up matrix */
	tMatrix.xx = (FT_Fixed)( cos( dAngle ) * 0x10000L );
	tMatrix.xy = (FT_Fixed)(-sin( dAngle ) * 0x10000L );
	tMatrix.yx = (FT_Fixed)( sin( dAngle ) * 0x10000L );
	tMatrix.yy = (FT_Fixed)( cos( dAngle ) * 0x10000L );
	
	/* 以笛卡尔坐标原点为参照点 */
	tPen.x = 0;
	tPen.y = 0;
	
	/* set transformation */
	FT_Set_Transform(g_tFace, &tMatrix, &tPen);

	/* load glyph image into the slot (erase previous one) */
	error = FT_Load_Char(g_tFace, dwCode, FT_LOAD_RENDER);
	if (error)
		;				  /* ignore errors */

	/* now, draw to our target surface (convert position) */
	//draw_bitmap(&tSlot->bitmap, tSlot->bitmap_left, iTargetHeight - tSlot->bitmap_top);

	/* 根据传入的需要绘制的起点，计算实际绘制的起点以及绘制范围的大小 */
	ptFontPara->iXLeft = iPenX + tSlot->bitmap_left;
	ptFontPara->iYTop  = iPenY - tSlot->bitmap_top;//?????
	ptFontPara->iXmax  = ptFontPara->iXLeft + tSlot->bitmap.width;
	ptFontPara->iYmax  = ptFontPara->iYTop + tSlot->bitmap.rows;

	/* increment pen position */
	ptFontPara->iNextOriginX = iPenX + tSlot->advance.x / 64;
	ptFontPara->iNextOriginY = iPenY;

	ptFontPara->pucBuffer = tSlot->bitmap.buffer;

	return 0;
}

int Freetype_Init(char *pcFileName, unsigned int font_size)
{
	FT_Error error;

	/* Initialize a new FreeType library object */
	error = FT_Init_FreeType(&g_tLibrary);
	if(error)
		return error;

	/* open a font by its pathname */
	error = FT_New_Face(g_tLibrary, pcFileName, 0, &g_tFace);
	if(error)
		return error;

#if 0
	/* use 50pt at 100dpi */
	error = FT_Set_Char_Size(face, 50 * 64, 0, 100, 0);/* set character size */
	/* error handling omitted */
#else
	error = FT_Set_Pixel_Sizes(g_tFace, font_size, 0);
#endif

	//show_image();
	
	return 0;
}

void Freetype_Exit(void)
{
	FT_Done_Face(g_tFace);
	FT_Done_FreeType(g_tLibrary);
}

static T_Font_Opr g_tFreetypeOpr = {
	.c_pFontName = "freetype",
	.Font_Init   = Freetype_Init,
	.Get_Bitmap  = Freetype_Get_Bitmap,
	.Font_Exit   = Freetype_Exit,
};

int Freetype_Opr_Init(void)
{
	return Font_Opr_Regisiter(&g_tFreetypeOpr);
}

