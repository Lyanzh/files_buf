#include "tinyOS.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL	0xE000ED04
#define NVIC_PENDSVSET	0x10000000
#define NVIC_SYSPRI2	0xE000ED22
#define NVIC_PENDSV_PRI	0x000000FF

#define MEM32(addr)		*(volatile unsigned long *)(addr)
#define MEM8(addr)		*(volatile unsigned char *)(addr)

// ��ǰ���񣺼�¼��ǰ���ĸ�������������
tTask *currentTask;

// ��һ���������е������ڽ��������л�ǰ�������úø�ֵ��Ȼ�������л������л���ж�ȡ��һ������Ϣ
tTask *nextTask;

tList taskTable[TINYOS_PRIO_COUNT];

// �������ȼ��ı��λ�ýṹ
tBitmap taskPrioBitmap;

// ��ʱ����
tList tTaskDelayList;

// ������������
static u32 schedLockCount;

/*
 * �ڽ���PendSV������ʱ��
 * ��1��xPSR��PC��LR��R12��R0��R3�Ѿ��ڴ���ջ�б����档
 * ��2������ģʽ�л����߳�ģʽ��
 * ��3��ջ������ջ��
 */
__asm void PendSV_Handler(void)
{
	IMPORT currentTask
	IMPORT nextTask

	MRS R0, PSP					/* ��ȡPSP��R0 */
	CBZ R0, PendSVHander_nosave	/* ����0����ת */

	/* ��������״̬ */
	STMDB R0!, {R4-R11}
	LDR R1, =currentTask
	LDR R1, [R1]
	STR R0, [R1]

PendSVHander_nosave
	/* currentTask = nextTask */
	LDR R0, =currentTask	/* currentTask�ĵ�ֵַ�浽R0    */
	LDR R1, =nextTask		/* nextTask�ĵ�ֵַ�浽R1       */
	LDR R2, [R1]			/* nextTask��ַָ������ݴ浽R2 */
	STR R2, [R0]			/* ��R2�����ݴ浽R0��ָ����ڴ浥Ԫ(��currentTask) */

	LDR R0, [R2]
	LDMIA R0!, {R4-R11}	/* ��ջ */

	MSR PSP, R0
	ORR LR, LR, #0x04
	BX LR
}

static void triggerPendSVC(void)
{
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void TaskRunFirst(void)
{
	__set_PSP(0);
	triggerPendSVC();
}

void TaskSwitch(void)
{
	triggerPendSVC();
}

tTask *TaskHighestReady(void)
{
	u32 highestPrio;
	tNode *node;
	highestPrio = BitmapGetFirstSet(&taskPrioBitmap);
	node = ListFirst(&taskTable[highestPrio]);
	return (tTask *)NodeParent(node, tTask, linkNode);
}

/* ����������Ϊ����״̬ */
void TaskSchedRdy(tTask *task)
{
	ListAddLast(&taskTable[task->prio], &(task->linkNode));
	BitmapSet(&taskPrioBitmap, task->prio);
}

/* ������Ӿ����б����Ƴ� */
void TaskSchedUnRdy(tTask *task)
{
	ListRemove(&taskTable[task->prio], &(task->linkNode));

	if (ListCount(&taskTable[task->prio]) == 0) {
		BitmapClear(&taskPrioBitmap, task->prio);
	}
}

void TaskSched(void)
{
	tTask *tempTask;
	u32 status = TaskEnterCritical();
	if (schedLockCount > 0) {
		TaskExitCritical(status);
		return;
	}

	tempTask = TaskHighestReady();
	if (tempTask != currentTask) {
		nextTask = tempTask;
		TaskSwitch();
	}
	
	TaskExitCritical(status);
}

void TaskDelayInit(void)
{
	ListInit(&tTaskDelayList);
}

/* �����������ʱ������ */
void TimeTaskWait(tTask *task, u32 ticks)
{
	task->delayTicks = ticks;
	ListAddLast(&tTaskDelayList, &(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAY;
}

/* ����ʱ���������ʱ�����л��� */
void TimeTaskWakeUp(tTask *task)
{
	ListRemove(&tTaskDelayList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAY;
}

/* ����ʱ���������ʱ�������Ƴ� */
void TimeTaskRemove(tTask *task)
{
	ListRemove(&tTaskDelayList, &(task->delayNode));
}

void TaskSysTickHandler(void)
{
	tNode *node;
	tTask *task;
	u32 status = TaskEnterCritical();

	// ������������delayTicks���������0�Ļ�����1
	for (node = tTaskDelayList.headNode.nextNode; node != &(tTaskDelayList.headNode); node = node->nextNode) {
		task = NodeParent(node, tTask, delayNode);
		if (--task->delayTicks == 0) {
			// ������񻹴��ڵȴ��¼���״̬��������¼��ȴ������л���
			if (task->waitEvent) {
				// ��ʱ����ϢΪ�գ��ȴ����Ϊ��ʱ
				EventRemoveTask(task, (void *)0, tErrorTimeout);
			}

			// ���������ʱ�������Ƴ�
			TimeTaskWakeUp(task);

			// ������ָ�������״̬
			TaskSchedRdy(task);
		}
	}

	// ����µ�ǰ�����ʱ��Ƭ�Ƿ��Ѿ�����
	if (--currentTask->slice == 0) {
        // �����ǰ�����л�����������Ļ�����ô�л�����һ������
        // �����ǽ���ǰ����Ӷ��е�ͷ���Ƴ������뵽β��
        // ��������ִ��tTaskSched()ʱ�ͻ��ͷ��ȡ���µ�����ȡ���µ�������Ϊ��ǰ��������
		if (ListCount(&taskTable[currentTask->prio]) > 0) {
			ListRemoveFirst(&taskTable[currentTask->prio]);
			ListAddLast(&taskTable[currentTask->prio], &currentTask->linkNode);

			// ���ü�����
			currentTask->slice = TINYOS_SLICE_MAX;
		}
	}

	TaskExitCritical(status);

	TaskSched();
}

/* �����ٽ��� */
u32 TaskEnterCritical(void)
{
	u32 primask = __get_PRIMASK();
	__disable_irq();
	return primask;
}

/* �˳��ٽ��� */
void TaskExitCritical(u32 status)
{
	__set_PRIMASK(status);
}

/* ��ʼ�� */
void TaskSchedInit(void)
{
	u32 i = 0;
	
	schedLockCount = 0;
	BitmapInit(&taskPrioBitmap);

	for (i = 0; i < TINYOS_PRIO_COUNT; i++) {
		ListInit(&taskTable[i]);
	}
}

/* ���������� */
void TaskSchedDisable(void)
{
	u32 status = TaskEnterCritical();
	
	if (schedLockCount < 255) {
		schedLockCount++;
	}

	TaskExitCritical(status);
}

/* ���������� */
void TaskSchedEnable(void)
{
	u32 status = TaskEnterCritical();

	if (schedLockCount > 0) {
		if (--schedLockCount == 0) {
			TaskSched();
		}
	}

	TaskExitCritical(status);
}

