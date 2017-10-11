#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library g_tLibrary;
FT_Face    g_tFace;
	
int Freetype_Get_Bitmap(unsigned int dwCode, PT_Font_Para ptFontPara)
{
	FT_GlyphSlot tSlot;
	FT_Matrix	 tMatrix;	/* transformation matrix */
	FT_Vector	 tPen;		/* untransformed origin  */
	
	double dAngle;
	
	tSlot = g_tFace->glyph;

	dAngle = (0 / 360) * 3.14159 * 2;	   /* use 0 degrees */

	/* set up matrix */
	tMatrix.xx = (FT_Fixed)( cos( dAngle ) * 0x10000L );
	tMatrix.xy = (FT_Fixed)(-sin( dAngle ) * 0x10000L );
	tMatrix.yx = (FT_Fixed)( sin( dAngle ) * 0x10000L );
	tMatrix.yy = (FT_Fixed)( cos( dAngle ) * 0x10000L );
	
	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	tPen.x = iX * 64;
	tPen.y = (iTargetHeight - iY) * 64;

	
		/* set transformation */
		FT_Set_Transform(g_tFace, &tMatrix, &tPen);

		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Char(g_tFace, dwCode, FT_LOAD_RENDER);
		if (error)
			continue;				  /* ignore errors */

		/* now, draw to our target surface (convert position) */
		draw_bitmap(&tSlot->bitmap, tSlot->bitmap_left, iTargetHeight - tSlot->bitmap_top);

		ptFontPara->iCurXres = tSlot->bitmap_left;
		ptFontPara->iCurYres = 
		

		/* increment pen position */
		tPen.x += tSlot->advance.x;
		tPen.y += tSlot->advance.y;
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
	error = FT_Set_Pixel_Size(g_tFace, font_size, 0);
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
	.Font_Init = Freetype_Init,
	.Font_Exit = Freetype_Exit,
};

int Freetype_Opr_Init(void)
{
	return Font_Opr_Regisiter(&g_tFreetypeOpr);
}
