#include "tinyOS.h"

void TaskInit(tTask *task, void (*entry)(void *), void *param, u32 prio, tTaskStack *stack)
{
	*(--stack) = (unsigned long)(1 << 24);	// XPSR, 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
	*(--stack) = (unsigned long)entry;		//R15(PC),程序的入口地址
	*(--stack) = (unsigned long)0x14;		//R14(LR), 任务不会通过return xxx结束自己，所以未用
	*(--stack) = (unsigned long)0x12;		//R12
	*(--stack) = (unsigned long)0x3;		//R3
	*(--stack) = (unsigned long)0x2;		//R2
	*(--stack) = (unsigned long)0x1;		//R1
	*(--stack) = (unsigned long)param;		//R0 = param, 传给任务的入口函数

	*(--stack) = (unsigned long)0x11;		//R11
	*(--stack) = (unsigned long)0x10;		//R10
	*(--stack) = (unsigned long)0x9;		//R9
	*(--stack) = (unsigned long)0x8;		//R8
	*(--stack) = (unsigned long)0x7;		//R7
	*(--stack) = (unsigned long)0x6;		//R6
	*(--stack) = (unsigned long)0x5;		//R5
	*(--stack) = (unsigned long)0x4;		//R4

	// 初始化任务的时间片计数
	task->slice = TINYOS_SLICE_MAX;

	// 保存最终的值
	task->stack = stack;
	
	task->delayTicks = 0;

	// 设置任务的优先级
	task->prio = prio;

	// 设置任务为就绪状态
	task->state = TINYOS_TASK_STATE_RDY;

	// 初始挂起次数为0
	task->suspendCount = 0;

	task->clean = (void(*)(void *))0;
	task->cleanParam = (void *)0;
	task->requestDeleteFlag = 0;

	// 初始化延时结点
	NodeInit(&(task->delayNode));

	// 初始化链接结点
	NodeInit(&(task->linkNode));
	ListAddLast(&taskTable[prio], &(task->linkNode));

	BitmapSet(&taskPrioBitmap, prio);
}

/* 挂起指定的任务 */
void TaskSuspend(tTask *task)
{
	u32 status = TaskEnterCritical();

	// 不允许对已经进入延时状态的任务挂起
	if (!(task->state & TINYOS_TASK_STATE_DELAY)) {
		// 增加挂起计数，仅当该任务被执行第一次挂起操作时，才考虑是否
        // 要执行任务切换操作
		if (++task->suspendCount <= 1) {
			task->state |= TINYOS_TASK_STATE_SUSPEND;

			TaskSchedUnRdy(task);

			// 当然，这个任务可能是自己，那么就切换到其它任务
			if (task == currentTask) {
				TaskSched();
			}
		}
	}

	TaskExitCritical(status);
}

void TaskWakeUp(tTask *task)
{
	u32 status = TaskEnterCritical();

	if (task->state & TINYOS_TASK_STATE_SUSPEND) {
		// 递减挂起计数，如果为0了，则清除挂起标志，同时设置进入就绪状态
		if (--task->suspendCount == 0) {
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;

			TaskSchedRdy(task);

			TaskSched();
		}
	}

	TaskExitCritical(status);
}

/*  设置任务被删除时调用的清理函数 */
void TaskSetCleanCallFunc(tTask *task, void (*clean)(void *param), void *param)
{
	task->clean = clean;
	task->cleanParam = param;
}

void TaskForceDelete(tTask *task)
{
	u32 status = TaskEnterCritical();

	if (task->state & TINYOS_TASK_STATE_DELAY) {
		TimeTaskRemove(task);
	}
	// 如果任务不处于挂起状态，那么就是就绪态，从就绪表中删除
	else if (!(task->state & TINYOS_TASK_STATE_SUSPEND)) {
		TaskSchedUnRdy(task);
	}

	if (task->clean) {
		task->clean(task->cleanParam);
	}

	// 如果删除的是自己，那么需要切换至另一个任务，所以执行一次任务调度
	if (currentTask == task) {
		TaskSched();
	}

	TaskExitCritical(status);
}

void TaskRequestDelete(tTask *task)
{
	u32 status = TaskEnterCritical();

	task->requestDeleteFlag = 1;

	TaskExitCritical(status);
}

u8 TaskIsRequestedDelete(void)
{
	u8 delete;

	u32 status = TaskEnterCritical();

	delete = currentTask->requestDeleteFlag;

	TaskExitCritical(status);

	return delete;
}

void TaskDeleteSelf(void)
{
	u32 status = TaskEnterCritical();

	TaskSchedUnRdy(currentTask);

	if (currentTask->clean) {
		currentTask->clean(currentTask->cleanParam);
	}

	TaskSched();

	TaskExitCritical(status);
}

void TaskGetInfo(tTask *task, tTaskInfo *info)
{
	u32 status = TaskEnterCritical();

	info->delayTicks = task->delayTicks;
	info->prio = task->prio;
	info->state = task->state;
	info->slice = task->slice;
	info->suspendCount = task->suspendCount;

	TaskExitCritical(status);
}

