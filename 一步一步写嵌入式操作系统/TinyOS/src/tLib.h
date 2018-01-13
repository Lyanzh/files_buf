#ifndef TLIB_H
#define TLIB_H

#include "global.h"

typedef struct
{
	u32 bitmap;
} tBitmap;

void BitmapInit(tBitmap *bitmap);
u32 BitmapPosCount(void);
void BitmapSet(tBitmap *bitmap, u32 pos);
void BitmapClear(tBitmap *bitmap, u32 pos);
u32 BitmapGetFirstSet(tBitmap *bitmap);

#endif
