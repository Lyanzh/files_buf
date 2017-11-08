#include "format_manager.h"

typedef struct tagBITMAPFILEHEADER 
{  
	unsigned short bfType;
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bfOffBits;
} BITMAPFILEHEADER;



