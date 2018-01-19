#include "tinyOS.h"

//初始化事件控制块
void EventInit(tEvent *event, tEventType type)
{
	event->type = type;
	ListInit(&event->waitList);
}

/*
** Descriptions         :   让指定在事件控制块上等待事件发生
** parameters           :   event 事件控制块
** parameters           :   task 等待事件发生的任务
** parameters           :   msg 事件消息存储的具体位置
** parameters           :   state 消息类型
** parameters           :   timeout 等待多长时间
*/
void EventWait(tEvent *event, tTask *task, void *msg, u32 state, u32 timeout)
{
	u32 status = TaskEnterCritical();

	task->state |= state;		// 标记任务处于等待某种事件的状态
	task->waitEvent = event;	// 设置任务等待的事件结构
	task->eventMsg = msg;		// 设置任务等待事件的消息存储位置 
								// 因有时候需要接受消息，所以需要接受区
	task->waitEventResult = tErrorNoError;	// 清空事件的等待结果

	TaskSchedUnRdy(task);		// 将任务从就绪队列中移除

	// 将任务插入到等待队列中
	ListAddLast(&event->waitList, &task->linkNode);

	// 如果发现有设置超时，在同时插入到延时队列中
	// 当时间到达时，由延时处理机制负责将任务从延时列表中移除，同时从事件列表中移除
	if (timeout) {
		TimeTaskWait(task, timeout);
	}

	TaskExitCritical(status);
}

/*
** Descriptions         :   从事件控制块中唤醒首个等待的任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
*/
tTask *EventWakeUp(tEvent *event, void *msg, u32 result)
{
	tNode *node;
	tTask *task;
	
	u32 status = TaskEnterCritical();

	// 取出等待队列中的第一个结点
	if ((node = ListRemoveFirst(&event->waitList)) != (tNode *)0) {
		// 转换为相应的任务结构
		task = (tTask *)NodeParent(node, tTask, linkNode);

		// 设置收到的消息、结构，清除相应的等待标志位
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->waitEventResult = result;
		task->state &= ~TINYOS_TASK_WAIT_MASK;

		// 任务申请了超时等待，这里检查下，将其从延时队列中移除
		if (task->delayTicks != 0) {
			TimeTaskWakeUp(task);
		}

		// 将任务加入就绪队列
		TaskSchedRdy(task);
	}

	TaskExitCritical(status);

	return task;
}

/*
** Function name        :   tEventRemoveTask
** Descriptions         :   将任务从其等待队列中强制移除
** parameters           :   task 待移除的任务
** parameters           :   result 告知事件的等待结果
** Returned value       :   无
*/
void EventRemoveTask(tTask *task, void *msg, u32 result)
{
	u32 status = TaskEnterCritical();

	// 将任务从所在的等待队列中移除
	// 注意，这里没有检查waitEvent是否为空。既然是从事件中移除，那么认为就不可能为空
	ListRemove(&task->waitEvent->waitList, &task->linkNode);

	// 设置收到的消息、结构，清除相应的等待标志位
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;
	task->waitEventResult = result;
	task->state &= ~TINYOS_TASK_WAIT_MASK;

	TaskExitCritical(status);
}

/*
** Function name        :   tEventRemoveAll
** Descriptions         :   清除所有等待中的任务，将事件发送给所有任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
*/
u32 EventRemoveAll(tEvent *event, void *msg, u32 result)
{
	tNode *node;
	u32 count;

	tTask *task;

	u32 status = TaskEnterCritical();

	// 获取等待中的任务数量
	count = ListCount(&event->waitList);

	// 遍历所有等待中的任务
	while ((node = ListRemoveFirst(&event->waitList)) != (tNode *)0) {
		task = (tTask *)NodeParent(node, tTask, linkNode);

		// 设置收到的消息、结构，清除相应的等待标志位
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->waitEventResult = result;
		task->state &= ~TINYOS_TASK_WAIT_MASK;

		// 如果任务申请了超时等待，将其从延时队列中移除
		if (task->delayTicks != 0) {
			TimeTaskWakeUp(task);
		}

		// 将任务加入就绪队列
		TaskSchedRdy(task);
	}

	TaskExitCritical(status);

	return count;
}

/*
** Function name        :   tEventWaitCount
** Descriptions         :   事件控制块中等待的任务数量
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
*/
u32 EventWaitCount(tEvent *event)
{
	u32 count = 0;

	u32 status = TaskEnterCritical();

	count = ListCount(&event->waitList);

	TaskExitCritical(status);

	return count;
}

