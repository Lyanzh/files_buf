#ifndef EVENT_H
#define EVENT_H

#include "global.h"
#include "tLib.h"
#include "task.h"

// 前置声明
typedef struct _tTask tTask;

// Event类型
typedef enum _tEventType {
	tEventTypeUnknown	= 0,
} tEventType;

// Event控制结构
typedef struct _tEvent {
	tEventType type;
	tList waitList;
} tEvent;

void EventInit(tEvent *event, tEventType type);
void EventWait(tEvent *event, tTask *task, void *msg, u32 state, u32 timeout);
tTask *EventWakeUp(tEvent *event, void *msg, u32 result);
void EventRemoveTask(tTask *task, void *msg, u32 result);
u32 EventRemoveAll(tEvent *event, void *msg, u32 result);
u32 EventWaitCount(tEvent *event);

#endif
