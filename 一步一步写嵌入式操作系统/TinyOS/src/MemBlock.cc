#include "Memblock.h"
#include "tinyOS.h"

/*
 * Function name        :   tMemBlockInit
 * Descriptions         :   初始化存储控制块
 * parameters           :   memBlock 等待初始化的存储控制块
 * parameters           :   memStart 存储区的起始地址
 * parameters           :   blockSize 每个块的大小
 * parameters           :   blockCnt 总的块数量
 * Returned value       :   唤醒的任务数量
 */
void MemBlockInit(tMemBlock *memBlock, u8 *memStart, u32 blockSize, u32 blockCnt)
{
	u8 *memBlockStart = (u8 *)memStart;
	u8 *memBlcokEnd = memBlockStart + blockSize * blockCnt;

	if (blockSize < sizeof(tNode)) {
		return;
	}

	EventInit(&memBlock->event, tEventTypeMemBlock);

	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;

	ListInit(&memBlock->blockList);
	while (memBlockStart < memBlcokEnd) {
		NodeInit((tNode *)memBlockStart);
		ListAddLast(&memBlock->blockList, (tNode *)memBlockStart);

		memBlockStart += blockSize;
	}
}
/*
** Function name        :   tMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   memBlock 等待的存储块
** parameters			:   mem 存储块存储的地址
** parameters           :   waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
*/
u32 MemBlockWait(tMemBlock *memBlock, u8 **mem, u32 waitTicks)
{
	u32 status = TaskEnterCritical();

	// 首先检查是否有空闲的存储块
	if (ListCount(&memBlock->blockList) > 0) {
		// 如果有的话，取出一个
		*mem = (u8 *)ListRemoveFirst(&memBlock->blockList);
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		// 否则将任务插入事件队列中
		EventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);
		TaskExitCritical(status);

		// 最后再执行一次事件调度，以便于切换到其它任务
		TaskSched();

		// 当切换回来时，从tTask中取出获得的消息
		*mem = currentTask->eventMsg;

		// 取出等待结果
		return currentTask->waitEventResult;
	}
}

/*
** Function name        :   tMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   memBlock 等待的存储块
** parameters			:   mem 存储块存储的地址
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
*/
u32 MemBlockNoWaitGet(tMemBlock *memBlock, void **mem)
{
	u32 status = TaskEnterCritical();

	// 首先检查是否有空闲的存储块
	if (ListCount(&memBlock->blockList) > 0) {
		// 如果有的话，取出一个
		*mem = (u8 *)ListRemoveFirst(&memBlock->blockList);
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		// 否则返回资源不可用
		TaskExitCritical(status);
		return tErrorResourceUnavalible;
	}
}

/*
** Function name        :   tMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   memBlock 操作的信号量
** Returned value       :   无
*/
void MemBlockNotify(tMemBlock *memBlock, u8 *mem)
{
	u32 status = TaskEnterCritical();

	// 检查是否有任务等待
	if (EventWaitCount(&memBlock->event) > 0) {
		// 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
		tTask *task = EventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);

		// 如果这个任务的优先级更高，就执行调度，切换过去
		if (task->prio < currentTask->prio) {
			TaskSched();
		}
	} else {
		// 如果没有任务等待的话，将存储块插入到队列中
		ListAddLast(&memBlock->blockList, (tNode *)mem);
	}

	TaskExitCritical(status);
}

