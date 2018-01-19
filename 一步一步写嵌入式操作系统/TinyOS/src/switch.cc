#include "tinyOS.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL	0xE000ED04
#define NVIC_PENDSVSET	0x10000000
#define NVIC_SYSPRI2	0xE000ED22
#define NVIC_PENDSV_PRI	0x000000FF

#define MEM32(addr)		*(volatile unsigned long *)(addr)
#define MEM8(addr)		*(volatile unsigned char *)(addr)

// 当前任务：记录当前是哪个任务正在运行
tTask *currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
tTask *nextTask;

tList taskTable[TINYOS_PRIO_COUNT];

// 任务优先级的标记位置结构
tBitmap taskPrioBitmap;

// 延时队列
tList tTaskDelayList;

// 调度锁计数器
static u32 schedLockCount;

/*
 * 在进入PendSV处理函数时：
 * （1）xPSR、PC、LR、R12、R0～R3已经在处理栈中被保存。
 * （2）处理模式切换到线程模式。
 * （3）栈是主堆栈。
 */
__asm void PendSV_Handler(void)
{
	IMPORT currentTask
	IMPORT nextTask

	MRS R0, PSP					/* 获取PSP到R0 */
	CBZ R0, PendSVHander_nosave	/* 等于0则跳转 */

	/* 保存任务状态 */
	STMDB R0!, {R4-R11}
	LDR R1, =currentTask
	LDR R1, [R1]
	STR R0, [R1]

PendSVHander_nosave
	/* currentTask = nextTask */
	LDR R0, =currentTask	/* currentTask的地址值存到R0    */
	LDR R1, =nextTask		/* nextTask的地址值存到R1       */
	LDR R2, [R1]			/* nextTask地址指向的内容存到R2 */
	STR R2, [R0]			/* 把R2的内容存到R0所指向的内存单元(即currentTask) */

	LDR R0, [R2]
	LDMIA R0!, {R4-R11}	/* 出栈 */

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

/* 将任务设置为就绪状态 */
void TaskSchedRdy(tTask *task)
{
	ListAddLast(&taskTable[task->prio], &(task->linkNode));
	BitmapSet(&taskPrioBitmap, task->prio);
}

/* 将任务从就绪列表中移除 */
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

/* 将任务加入延时队列中 */
void TimeTaskWait(tTask *task, u32 ticks)
{
	task->delayTicks = ticks;
	ListAddLast(&tTaskDelayList, &(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAY;
}

/* 将延时的任务从延时队列中唤醒 */
void TimeTaskWakeUp(tTask *task)
{
	ListRemove(&tTaskDelayList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAY;
}

/* 将延时的任务从延时队列中移除 */
void TimeTaskRemove(tTask *task)
{
	ListRemove(&tTaskDelayList, &(task->delayNode));
}

void TaskSysTickHandler(void)
{
	tNode *node;
	tTask *task;
	u32 status = TaskEnterCritical();

	// 检查所有任务的delayTicks数，如果不0的话，减1
	for (node = tTaskDelayList.headNode.nextNode; node != &(tTaskDelayList.headNode); node = node->nextNode) {
		task = NodeParent(node, tTask, delayNode);
		if (--task->delayTicks == 0) {
			// 如果任务还处于等待事件的状态，则将其从事件等待队列中唤醒
			if (task->waitEvent) {
				// 此时，消息为空，等待结果为超时
				EventRemoveTask(task, (void *)0, tErrorTimeout);
			}

			// 将任务从延时队列中移除
			TimeTaskWakeUp(task);

			// 将任务恢复到就绪状态
			TaskSchedRdy(task);
		}
	}

	// 检查下当前任务的时间片是否已经到了
	if (--currentTask->slice == 0) {
        // 如果当前任务中还有其它任务的话，那么切换到下一个任务
        // 方法是将当前任务从队列的头部移除，插入到尾部
        // 这样后面执行tTaskSched()时就会从头部取出新的任务取出新的任务作为当前任务运行
		if (ListCount(&taskTable[currentTask->prio]) > 0) {
			ListRemoveFirst(&taskTable[currentTask->prio]);
			ListAddLast(&taskTable[currentTask->prio], &currentTask->linkNode);

			// 重置计数器
			currentTask->slice = TINYOS_SLICE_MAX;
		}
	}

	TaskExitCritical(status);

	TaskSched();
}

/* 进入临界区 */
u32 TaskEnterCritical(void)
{
	u32 primask = __get_PRIMASK();
	__disable_irq();
	return primask;
}

/* 退出临界区 */
void TaskExitCritical(u32 status)
{
	__set_PRIMASK(status);
}

/* 初始化 */
void TaskSchedInit(void)
{
	u32 i = 0;
	
	schedLockCount = 0;
	BitmapInit(&taskPrioBitmap);

	for (i = 0; i < TINYOS_PRIO_COUNT; i++) {
		ListInit(&taskTable[i]);
	}
}

/* 调度锁上锁 */
void TaskSchedDisable(void)
{
	u32 status = TaskEnterCritical();
	
	if (schedLockCount < 255) {
		schedLockCount++;
	}

	TaskExitCritical(status);
}

/* 调度锁解锁 */
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

