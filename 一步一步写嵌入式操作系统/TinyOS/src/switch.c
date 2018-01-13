#include "tinyOS.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL	0xE000ED04
#define NVIC_PENDSVSET	0x10000000
#define NVIC_SYSPRI2	0xE000ED22
#define NVIC_PENDSV_PRI	0x000000FF

#define MEM32(addr)		*(volatile unsigned long *)(addr)
#define MEM8(addr)		*(volatile unsigned char *)(addr)

tTask *currentTask;
tTask *nextTask;
tTask *taskTable[TINYOS_PRIO_COUNT];
tBitmap taskPrioBitmap;
tTask *idleTask;

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
	highestPrio = BitmapGetFirstSet(&taskPrioBitmap);
	return taskTable[highestPrio];
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

void TaskInit(tTask *task, void (*entry)(void *), void *param, u32 prio, tTaskStack *stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;//R15, PC
	*(--stack) = (unsigned long)0x14;//R14
	*(--stack) = (unsigned long)0x13;//R13
	*(--stack) = (unsigned long)0x3;//R3
	*(--stack) = (unsigned long)0x2;//R2
	*(--stack) = (unsigned long)0x1;//R1
	*(--stack) = (unsigned long)param;//R0

	*(--stack) = (unsigned long)0x11;//R11
	*(--stack) = (unsigned long)0x10;//R10
	*(--stack) = (unsigned long)0x9;//R9
	*(--stack) = (unsigned long)0x8;//R8
	*(--stack) = (unsigned long)0x7;//R7
	*(--stack) = (unsigned long)0x6;//R6
	*(--stack) = (unsigned long)0x5;//R5
	*(--stack) = (unsigned long)0x4;//R4

	
	task->stack = stack;
	task->delayTicks = 0;
	task->prio = prio;

	taskTable[prio] = task;
	BitmapSet(&taskPrioBitmap, prio);
}

void SetSysTickPeriod(u32 ms)
{
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
					SysTick_CTRL_TICKINT_Msk |
					SysTick_CTRL_ENABLE_Msk;
}

void TaskSysTickHandler(void)
{
	u32 i;
	u32 status = TaskEnterCritical();
	for (i = 0; i < TINYOS_PRIO_COUNT; i++) {
		if (taskTable[i]->delayTicks > 0) {
			taskTable[i]->delayTicks--;
		} else {
			BitmapSet(&taskPrioBitmap, i);
		}
	}

	TaskExitCritical(status);

	TaskSched();
}

void SysTick_Handler(void)
{
	TaskSysTickHandler();
}

void taskDelay(u32 cnt)
{
	u32 status = TaskEnterCritical();
	currentTask->delayTicks = cnt;
	BitmapClear(&taskPrioBitmap, currentTask->prio);
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
	schedLockCount = 0;
	BitmapInit(&taskPrioBitmap);
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

