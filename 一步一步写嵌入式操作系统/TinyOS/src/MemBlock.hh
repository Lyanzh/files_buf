#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include "tConfig.h"
#include "event.h"

typedef struct _tMemBlock
{
	// �¼����ƿ�
	tEvent event;

	// �洢����׵�ַ
	void *memStart;

	// ÿ���洢��Ĵ�С
	u32 blockSize;

	// �ܵĴ洢��ĸ���
	u32 maxCount;

	// �洢���б�
	tList blockList;
} tMemBlock;

void MemBlockInit(tMemBlock *memBlock, u8 *memStart, u32 blockSize, u32 blockCnt);
u32 MemBlockWait(tMemBlock *memBlock, u8 **mem, u32 waitTicks);
u32 MemBlockNoWaitGet(tMemBlock *memBlock, void **mem);
void MemBlockNotify(tMemBlock *memBlock, u8 *mem);

#endif

