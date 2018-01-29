#include "tinyOS.h"

void MboxInit(tMbox *mbox, void **msgBuffer, u32 maxCount)
{
	EventInit(&mbox->event, tEventTypeMbox);

	mbox->msgBuffer = msgBuffer;
	mbox->maxCount = maxCount;
	mbox->read = 0;
	mbox->write = 0;
	mbox->count = 0;
}

u32 MboxWait(tMbox *mbox, void **msg, u32 waitTicks)
{
	u32 status = TaskEnterCritical();

	if (mbox->count > 0) {
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read++];
		if (mbox->read >= mbox->maxCount) {
			mbox->read = 0;
		}
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		EventWait(&mbox->event, currentTask, (void *)0, tEventTypeMbox, waitTicks);
		TaskExitCritical(status);

		TaskSched();

		*msg = currentTask->eventMsg;
		return currentTask->waitEventResult;
	}
}

u32 MboxNoWaitGet(tMbox *mbox, void **msg)
{
	u32 status = TaskEnterCritical();

	if (mbox->count > 0) {
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read++];
		if (mbox->read >= mbox->maxCount) {
			mbox->read = 0;
		}
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		TaskExitCritical(status);
		return tErrorResourceUnavalible;
	}
}

u32 MboxNotify(tMbox *mbox, void *msg, u32 notifyOption)
{
	tTask *task;
	
	u32 status = TaskEnterCritical();

	if (EventWaitCount(&mbox->event) > 0) {
		task = EventWakeUp(&mbox->event, (void *)msg, tErrorNoError);
		if (task->prio < currentTask->prio) {
			TaskSched();
		}
	} else {
		if (mbox->count >= mbox->maxCount) {
			TaskExitCritical(status);
			return tErrorResourceFull;
		}

		if (notifyOption & tMboxSendFront) {
			if (mbox->read <= 0) {
				mbox->read = mbox->maxCount - 1;
			} else {
				--mbox->read;
			}
			mbox->msgBuffer[mbox->read] = msg;
		} else {
			mbox->msgBuffer[mbox->write++] = msg;
			if (mbox->write > mbox->maxCount) {
				mbox->write = 0;
			}
		}

		mbox->count++;
	}

	TaskExitCritical(status);

	return tErrorNoError;
}

// 清空消息邮箱
void MboxFlush(tMbox *mbox)
{
	u32 status = TaskEnterCritical();

	// 如果队列中有任务在等待，说明邮箱已经是空的了，无需再清空
	if (EventWaitCount(&mbox->event) == 0) {
		mbox->read = 0;
		mbox->write = 0;
		mbox->count = 0;
	}

	TaskExitCritical(status);
}

// 删除消息邮箱
u32 MboxDestroy(tMbox *mbox)
{
	u32 status = TaskEnterCritical();

	u32 count = EventRemoveAll(&mbox->event, (void *)0, tErrorDel);

	TaskExitCritical(status);

	if (count > 0) {
		TaskSched();
	}

	return count;
}

void MboxGetInfo(tMbox *mbox, tMboxInfo *info)
{
	u32 status = TaskEnterCritical();

	info->count = mbox->count;
	info->maxCount = mbox->maxCount;
	info->taskCount = EventWaitCount(&mbox->event);

	TaskExitCritical(status);
}

