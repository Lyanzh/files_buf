#ifndef TINYOS_H
#define TINYOS_H

#include "global.h"
#include "tConfig.h"
#include "task.h"
#include "tLib.h"
#include "event.h"

// tinyOS的错误码
typedef enum _tError {
	tErrorNoError = 0,					// 没有错误
	tErrorTimeout,						// 等待超时
} tError;

extern tTask *currentTask;
extern tTask *nextTask;
extern tList taskTable[TINYOS_PRIO_COUNT];
extern tBitmap taskPrioBitmap;

void TaskRunFirst(void);
void TaskSchedRdy(tTask *task);
tTask *TaskHighestReady(void);
void TaskSchedUnRdy(tTask *task);
void TaskSched(void);
void TaskDelayInit(void);
void taskDelay(u32 cnt);
void TimeTaskWait(tTask *task, u32 ticks);
void TimeTaskWakeUp(tTask *task);
void TimeTaskRemove(tTask *task);
void SetSysTickPeriod(u32 ms);
u32 TaskEnterCritical(void);
void TaskExitCritical(u32 status);
void TaskSchedInit(void);
void TaskSchedDisable(void);
void TaskSchedEnable(void);
void TaskSysTickHandler(void);
void InitApp(void);

#endif
