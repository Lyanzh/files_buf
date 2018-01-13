#include "tLib.h"

void BitmapInit(tBitmap *bitmap)
{
	bitmap->bitmap = 0;
}

u32 BitmapPosCount(void)
{
	return 32;
}

void BitmapSet(tBitmap *bitmap, u32 pos)
{
	bitmap->bitmap |= (1 << pos);
}

void BitmapClear(tBitmap *bitmap, u32 pos)
{
	bitmap->bitmap &= ~(1 << pos);
}

u32 BitmapGetFirstSet(tBitmap *bitmap)
{
	static const u8 quickFindTable[] = {
		0xFF, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		6,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		7,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		6,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};

	if (bitmap->bitmap & 0xFF) {
		return quickFindTable[bitmap->bitmap & 0xFF];
	} else if (bitmap->bitmap & 0xFF00) {
		return quickFindTable[(bitmap->bitmap >> 8) & 0xFF] + 8;
	} else if (bitmap->bitmap & 0xFF0000) {
		return quickFindTable[(bitmap->bitmap >> 16) & 0xFF] + 16;
	} else if (bitmap->bitmap & 0xFF000000) {
		return quickFindTable[(bitmap->bitmap >> 24) & 0xFF] + 24;
	} else {
		return BitmapPosCount();
	}
}

