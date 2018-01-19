#ifndef TASK_H
#define TASK_H

#include "global.h"
#include "tConfig.h"
#include "tLib.h"
#include "event.h"

#define TINYOS_TASK_STATE_RDY			0
#define TINYOS_TASK_STATE_DELAY			(1 << 1)
#define TINYOS_TASK_STATE_SUSPEND		(1 << 2)

#define TINYOS_TASK_WAIT_MASK			0xFF00

// 前置声明
typedef struct _tEvent tEvent;

typedef u32 tTaskStack;

typedef struct _tTask {
	tTaskStack *stack;

	// 连接结点
	tNode linkNode;

	// 任务延时计数器
	u32 delayTicks;

	// 延时结点：通过delayNode就可以将tTask放置到延时队列中
	tNode delayNode;

	// 任务的优先级
	u32 prio;

	// 任务当前状态
	u32 state;

	// 当前剩余的时间片
	u32 slice;

	// 被挂起的次数
	u32 suspendCount;

	// 任务被删除时调用的清理函数
	void (*clean)(void *param);

	// 传递给清理函数的参数
	void *cleanParam;

	// 请求删除标志，非0表示请求删除
	u8 requestDeleteFlag;

	// 任务正在等待的事件类型
	tEvent *waitEvent;

	// 等待事件的消息存储位置
	void *eventMsg;

	// 等待事件的结果
	u32 waitEventResult;
} tTask;

typedef struct _tTaskInfo {
	u32 delayTicks;
	u32 prio;
	u32 state;
	u32 slice;
	u32 suspendCount;
} tTaskInfo;

void TaskInit(tTask *task, void (*entry)(void *), void *param, u32 prio, tTaskStack *stack);
void TaskSuspend(tTask *task);
void TaskWakeUp(tTask *task);
void TaskSetCleanCallFunc(tTask *task, void (*clean)(void *param), void *param);
void TaskForceDelete(tTask *task);
void TaskRequestDelete(tTask *task);
u8 TaskIsRequestedDelete(void);
void TaskDeleteSelf(void);
void TaskGetInfo(tTask *task, tTaskInfo *info);

#endif
