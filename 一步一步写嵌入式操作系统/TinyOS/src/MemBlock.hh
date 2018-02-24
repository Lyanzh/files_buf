#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include "tConfig.h"
#include "event.h"

typedef struct _tMemBlock
{
	// 事件控制块
	tEvent event;

	// 存储块的首地址
	void *memStart;

	// 每个存储块的大小
	u32 blockSize;

	// 总的存储块的个数
	u32 maxCount;

	// 存储块列表
	tList blockList;
} tMemBlock;

void MemBlockInit(tMemBlock *memBlock, u8 *memStart, u32 blockSize, u32 blockCnt);
u32 MemBlockWait(tMemBlock *memBlock, u8 **mem, u32 waitTicks);
u32 MemBlockNoWaitGet(tMemBlock *memBlock, void **mem);
void MemBlockNotify(tMemBlock *memBlock, u8 *mem);

#endif

