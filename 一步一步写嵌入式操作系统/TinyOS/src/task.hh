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

// ǰ������
typedef struct _tEvent tEvent;

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

	// �������ڵȴ����¼�����
	tEvent *waitEvent;

	// �ȴ��¼�����Ϣ�洢λ��
	void *eventMsg;

	// �ȴ��¼��Ľ��
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
