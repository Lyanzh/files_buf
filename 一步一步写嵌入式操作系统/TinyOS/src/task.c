#include "tinyOS.h"

void TaskInit(tTask *task, void (*entry)(void *), void *param, u32 prio, tTaskStack *stack)
{
	*(--stack) = (unsigned long)(1 << 24);	// XPSR, ������Thumbģʽ���ָ���Thumb״̬����ARM״̬����
	*(--stack) = (unsigned long)entry;		//R15(PC),�������ڵ�ַ
	*(--stack) = (unsigned long)0x14;		//R14(LR), ���񲻻�ͨ��return xxx�����Լ�������δ��
	*(--stack) = (unsigned long)0x12;		//R12
	*(--stack) = (unsigned long)0x3;		//R3
	*(--stack) = (unsigned long)0x2;		//R2
	*(--stack) = (unsigned long)0x1;		//R1
	*(--stack) = (unsigned long)param;		//R0 = param, �����������ں���

	*(--stack) = (unsigned long)0x11;		//R11
	*(--stack) = (unsigned long)0x10;		//R10
	*(--stack) = (unsigned long)0x9;		//R9
	*(--stack) = (unsigned long)0x8;		//R8
	*(--stack) = (unsigned long)0x7;		//R7
	*(--stack) = (unsigned long)0x6;		//R6
	*(--stack) = (unsigned long)0x5;		//R5
	*(--stack) = (unsigned long)0x4;		//R4

	// ��ʼ�������ʱ��Ƭ����
	task->slice = TINYOS_SLICE_MAX;

	// �������յ�ֵ
	task->stack = stack;
	
	task->delayTicks = 0;

	// ������������ȼ�
	task->prio = prio;

	// ��������Ϊ����״̬
	task->state = TINYOS_TASK_STATE_RDY;

	// ��ʼ�������Ϊ0
	task->suspendCount = 0;

	task->clean = (void(*)(void *))0;
	task->cleanParam = (void *)0;
	task->requestDeleteFlag = 0;

	// ��ʼ����ʱ���
	NodeInit(&(task->delayNode));

	// ��ʼ�����ӽ��
	NodeInit(&(task->linkNode));
	ListAddLast(&taskTable[prio], &(task->linkNode));

	BitmapSet(&taskPrioBitmap, prio);
}

/* ����ָ�������� */
void TaskSuspend(tTask *task)
{
	u32 status = TaskEnterCritical();

	// ��������Ѿ�������ʱ״̬���������
	if (!(task->state & TINYOS_TASK_STATE_DELAY)) {
		// ���ӹ������������������ִ�е�һ�ι������ʱ���ſ����Ƿ�
        // Ҫִ�������л�����
		if (++task->suspendCount <= 1) {
			task->state |= TINYOS_TASK_STATE_SUSPEND;

			TaskSchedUnRdy(task);

			// ��Ȼ���������������Լ�����ô���л�����������
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
		// �ݼ�������������Ϊ0�ˣ�����������־��ͬʱ���ý������״̬
		if (--task->suspendCount == 0) {
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;

			TaskSchedRdy(task);

			TaskSched();
		}
	}

	TaskExitCritical(status);
}

/*  ��������ɾ��ʱ���õ������� */
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
	// ������񲻴��ڹ���״̬����ô���Ǿ���̬���Ӿ�������ɾ��
	else if (!(task->state & TINYOS_TASK_STATE_SUSPEND)) {
		TaskSchedUnRdy(task);
	}

	if (task->clean) {
		task->clean(task->cleanParam);
	}

	// ���ɾ�������Լ�����ô��Ҫ�л�����һ����������ִ��һ���������
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

