#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library g_ftLibrary;
FT_Face    g_ftFace;
	
int Freetype_Get_Bitmap(char text, int iTargetHeight, int iX, int iY)
{
	FT_GlyphSlot ftSlot;
	FT_Matrix	 ftMatrix;	/* transformation matrix */
	FT_Vector	 ftPen;		/* untransformed origin  */
	
	double dAngle;
	
	ftSlot = g_ftFace->glyph;

	dAngle = (0 / 360) * 3.14159 * 2;	   /* use 0 degrees */

	/* set up matrix */
	ftMatrix.xx = (FT_Fixed)( cos( dAngle ) * 0x10000L );
	ftMatrix.xy = (FT_Fixed)(-sin( dAngle ) * 0x10000L );
	ftMatrix.yx = (FT_Fixed)( sin( dAngle ) * 0x10000L );
	ftMatrix.yy = (FT_Fixed)( cos( dAngle ) * 0x10000L );
	
	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	ftPen.x = iX * 64;
	ftPen.y = (iTargetHeight - iY) * 64;

	
		/* set transformation */
		FT_Set_Transform(g_ftFace, &ftMatrix, &ftPen);

		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Char(g_ftFace, text, FT_LOAD_RENDER);
		if (error)
			continue;				  /* ignore errors */

		/* now, draw to our target surface (convert position) */
		draw_bitmap(&ftSlot->bitmap, ftSlot->bitmap_left, iTargetHeight - ftSlot->bitmap_top);

		/* increment pen position */
		ftPen.x += ftSlot->advance.x;
		ftPen.y += ftSlot->advance.y;
}

int Freetype_Init(char *filename, unsigned int font_size)
{
	FT_Error error;

	/* Initialize a new FreeType library object */
	error = FT_Init_FreeType(&g_ftLibrary);
	if(error)
		return error;

	/* open a font by its pathname */
	error = FT_New_Face(g_ftLibrary, filename, 0, &g_ftFace);
	if(error)
		return error;

#if 0
	/* use 50pt at 100dpi */
	error = FT_Set_Char_Size(face, 50 * 64, 0, 100, 0);/* set character size */
	/* error handling omitted */
#else
	error = FT_Set_Pixel_Size(g_ftFace, font_size, 0);
#endif

	//show_image();
	
	return 0;
}

void Freetype_Exit(void)
{
	FT_Done_Face(g_ftFace);
	FT_Done_FreeType(g_ftLibrary);
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
