#ifndef TINYOS_H
#define TINYOS_H

#include "global.h"
#include "tConfig.h"
#include "tLib.h"

typedef u32 tTaskStack;

typedef struct _tTask {
	tTaskStack *stack;
	u32 delayTicks;
	u32 prio;
} tTask;

extern tTask *currentTask;
extern tTask *nextTask;
extern tTask *taskTable[TINYOS_PRIO_COUNT];
extern tTask *idleTask;

void TaskInit(tTask *task, void (*entry)(void *), void *param, u32 prio, tTaskStack *stack);
void TaskRunFirst(void);
void TaskSwitch(void);
void taskDelay(u32 cnt);
void SetSysTickPeriod(u32 ms);
u32 TaskEnterCritical(void);
void TaskExitCritical(u32 status);
void TaskSchedInit(void);
void TaskSchedDisable(void);
void TaskSchedEnable(void);

#endif
