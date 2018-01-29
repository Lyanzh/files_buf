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

		// �����ڵȴ���ʱ���߼�������ʱ��ִ�л᷵�ص����Ȼ��ȡ���ȴ����
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

	// �ȴ���������������Щ��������ȼ����ܱȵ�ǰ��������ȼ��ߣ�ִ��һ�ε���
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

