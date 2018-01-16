#ifndef TINYOS_H
#define TINYOS_H

#include "global.h"
#include "tConfig.h"
#include "tLib.h"

#define TINYOS_TASK_STATE_RDY			0
#define TINYOS_TASK_STATE_DELAY			(1 << 1)
#define TINYOS_TASK_STATE_SUSPEND		(1 << 2)

typedef u32 tTaskStack;

typedef struct _tTask {
	tTaskStack *stack;

	// ���ӽ��
	tNode linkNode;

	// ������ʱ������
	u32 delayTicks;

	// ��ʱ��㣺ͨ��delayNode�Ϳ��Խ�tTask���õ���ʱ������
	tNode delayNode;

	// ��������ȼ�
	u32 prio;

	// ����ǰ״̬
	u32 state;

	// ��ǰʣ���ʱ��Ƭ
	u32 slice;

	// ������Ĵ���
	u32 suspendCount;

	// ����ɾ��ʱ���õ�������
	void (*clean)(void *param);

	// ���ݸ��������Ĳ���
	void *cleanParam;

	// ����ɾ����־����0��ʾ����ɾ��
	u8 requestDeleteFlag;
} tTask;

typedef struct _tTaskInfo {
	u32 delayTicks;
	u32 prio;
	u32 state;
	u32 slice;
	u32 suspendCount;
} tTaskInfo;

extern tTask *currentTask;
extern tTask *nextTask;
extern tList taskTable[TINYOS_PRIO_COUNT];
extern tBitmap taskPrioBitmap;

void TaskInit(tTask *task, void (*entry)(void *), void *param, u32 prio, tTaskStack *stack);
void TaskRunFirst(void);
void TaskSuspend(tTask *task);
void TaskWakeUp(tTask *task);
void TaskSchedRdy(tTask *task);
tTask *TaskHighestReady(void);
void TaskSchedUnRdy(tTask *task);
void TaskSched(void);
void TaskDelayInit(void);
void taskDelay(u32 cnt);
void TimeTaskWait(tTask *task, u32 ticks);
void TimeTaskWakeUp(tTask *task);
void TimeTaskRemove(tTask *task);
void TaskSetCleanCallFunc(tTask *task, void (*clean)(void *param), void *param);
void TaskForceDelete(tTask *task);
void TaskRequestDelete(tTask *task);
u8 TaskIsRequestedDelete(void);
void TaskDeleteSelf(void);
void TaskGetInfo(tTask *task, tTaskInfo *info);
void SetSysTickPeriod(u32 ms);
u32 TaskEnterCritical(void);
void TaskExitCritical(u32 status);
void TaskSchedInit(void);
void TaskSchedDisable(void);
void TaskSchedEnable(void);
void TaskSysTickHandler(void);
void InitApp(void);

#endif
