#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONTDATAMAX 4096
static const unsigned char fontdata_8x16[FONTDATAMAX];

char *fbp;
unsigned int screen_bits_per_pixel = 0;
unsigned long screen_size = 0;
unsigned int line_size;
unsigned int pixel_size;

void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fbp + line_size * y + pixel_size * x;
	unsigned short *pen_16;
	unsigned int *pen_32;
	
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;
	
	unsigned int red, green, blue;
	
	switch(screen_bits_per_pixel)
	{
	case 8:
		*pen_8 = color;
		break;
	case 16:
		/* RGB565 */
		red = (color >> 16) & 0xff;
		green = (color >> 8) & 0xff;
		blue = (color >> 0) & 0xff;
		color = ((red >> 3) << 11) | ((green >> 2) << 5) | ((blue >> 3));
		*pen_16 = color;
		break;
	case 32:
		*pen_32 = color;
		break;
	default:
		printf("cannot surport %dbpp\n", screen_bits_per_pixel);
		break;
	}
}

void draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y)
{
	FT_Int i, j, p, q;
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;

	for (i = x, p = 0; i < x_max; i++, p++)
	{
		for (j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 || i >= 480 || j >= 272)
				continue;

			//image[j][i] |= bitmap->buffer[q * bitmap->width + p];
			lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
		}
	}
}

int main(int argc, char **argv)
{
	int fbfd = 0;
	struct fb_var_screeninfo vinfo;

	FT_Library library;
	FT_Face    face;
	FT_GlyphSlot slot;
	FT_Matrix	 matrix;				  /* transformation matrix */
	FT_Vector	 pen;					  /* untransformed origin  */
	FT_Error	 error;

	char* filename;
	char* text;
	double angle;
	int target_height;
	int n, num_chars;

	fbfd = open("/dev/fb0", O_RDWR);
	if(!fbfd)
	{
		printf("Error:cannot open framebuffer device.\n");
		return -1;
	}

	if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("Error:get variable information error.\n");
		return -1;
	}

	screen_bits_per_pixel = vinfo.bits_per_pixel;
	printf("screen_bits_per_pixel = %d\n", screen_bits_per_pixel);
	printf("screen x size = %d\n", vinfo.xres);
	printf("screen y size = %d\n", vinfo.yres);

	pixel_size = screen_bits_per_pixel / 8;	//pixel size in bytes
	line_size = vinfo.xres * pixel_size;	//line size in bytes
	screen_size = line_size * vinfo.yres;	//screen size in bytes
	printf("pixel_size = %d\n", pixel_size);
	printf("line_size = %d\n", line_size);
	printf("screen_size = %d\n", screen_size);
	
	//map the device to memory
	fbp = (char *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if((int)fbp == -1)
	{
		printf("Error:fail to map framebuffer device to memory.\n");
	}

	memset(fbp, 0, screen_size);

	//lcd_put_ascii(vinfo.xres / 2, vinfo.yres / 2, 'A');
	//lcd_put_str(472, 0, "abc");
	//lcd_put_str(0, 16, "efg");

	//use freetype to display
	filename	  = argv[1];						   /* first argument	 */
	text		  = argv[2];						   /* second argument	 */
	num_chars	  = strlen(text);
	angle		  = (0 / 360) * 3.14159 * 2;	   /* use 25 degrees	 */
	target_height = vinfo.yres;
	
	error = FT_Init_FreeType(&library);			   /* initialize library */
	/* error handling omitted */
	
	error = FT_New_Face(library, argv[1], 0, &face); /* create face object */
	/* error handling omitted */

#if 0
	/* use 50pt at 100dpi */
	error = FT_Set_Char_Size(face, 50 * 64, 0, 100, 0);/* set character size */
	/* error handling omitted */
#else
	error = FT_Set_Pixel_Sizes(face, 48, 0);
#endif

	slot = face->glyph;
	
	/* set up matrix */
	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
	
	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = 0 * 64;
	pen.y = (target_height - 48) * 64;
	
	for (n = 0; n < num_chars; n++)
	{
	  /* set transformation */
	  FT_Set_Transform(face, &matrix, &pen);
	
	  /* load glyph image into the slot (erase previous one) */
	  printf("Code = 0x%x\n", text[n]);
	  error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
	  if (error)
		continue;				  /* ignore errors */

	  printf("pitch = %d\n", slot->bitmap.pitch);
	  printf("bitmap_left = %d\n", slot->bitmap_left);
	  printf("bitmap_top = %d\n", slot->bitmap_top);
	
	  /* now, draw to our target surface (convert position) */
	  draw_bitmap(&slot->bitmap, slot->bitmap_left, target_height - slot->bitmap_top);
	
	  /* increment pen position */
	  pen.x += slot->advance.x;
	  pen.y += slot->advance.y;
	}
	
	//show_image();
	
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	munmap(fbp, screen_size);
	close(fbfd);
	return 0;
}

