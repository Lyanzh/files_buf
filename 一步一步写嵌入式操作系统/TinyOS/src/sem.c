#include "tinyOS.h"

void SemInit(tSem *sem, u32 startCount, u32 maxCount)
{
	EventInit(&sem->event, tEventTypeSem);

	sem->maxCount = maxCount;
	if (maxCount == 0) {
		sem->count = startCount;
	} else {
		sem->count = (startCount > maxCount)?maxCount:startCount;
	}
}

u32 SemWait(tSem *sem, u32 waitTicks)
{
	u32 status = TaskEnterCritical();

	if (sem->count > 0) {
		--sem->count;
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		EventWait(&sem->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
		TaskExitCritical(status);

		TaskSched();

		// 当由于等待超时或者计数可用时，执行会返回到这里，然后取出等待结果
		return currentTask->waitEventResult;
	}
}

void SemNotify(tSem *sem)
{
	tTask *task;
	u32 status = TaskEnterCritical();

	if (EventWaitCount(&sem->event) > 0) {
		task = EventWakeUp(&sem->event, (void *)0, tErrorNoError);

		if (task->prio < currentTask->prio) {
			TaskSched();
		}
	} else {
		++sem->count;

		if ((sem->maxCount != 0) && (sem->count > sem->maxCount)) {
			sem->count = sem->maxCount;
		}
	}
	
	TaskExitCritical(status);
}

u32 SemNoWaitGet(tSem *sem)
{
	u32 status = TaskEnterCritical();

	if (sem->count > 0) {
		--sem->count;
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		TaskExitCritical(status);
		return tErrorResourceUnavalible;
	}
}

u32 SemDestroy(tSem *sem)
{
	u32 status = TaskEnterCritical();

	u32 count = EventRemoveAll(&sem->event, (void *)0, tErrorDel);

	TaskExitCritical(status);

	// 等待队列中有任务，这些任务的优先级可能比当前任务的优先级高，执行一次调度
	if (count > 0) {
		TaskSched();
	}

	return count;
}

void SemGetInfo(tSem *sem, tSemInfo *info)
{
	u32 status = TaskEnterCritical();

	info->count = sem->count;
	info->maxCount = sem->maxCount;
	info->taskCount = EventWaitCount(&sem->event);

	TaskExitCritical(status);
}

