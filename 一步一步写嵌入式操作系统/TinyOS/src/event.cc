#include "tinyOS.h"

//��ʼ���¼����ƿ�
void EventInit(tEvent *event, tEventType type)
{
	event->type = type;
	ListInit(&event->waitList);
}

/*
** Descriptions         :   ��ָ�����¼����ƿ��ϵȴ��¼�����
** parameters           :   event �¼����ƿ�
** parameters           :   task �ȴ��¼�����������
** parameters           :   msg �¼���Ϣ�洢�ľ���λ��
** parameters           :   state ��Ϣ����
** parameters           :   timeout �ȴ��೤ʱ��
*/
void EventWait(tEvent *event, tTask *task, void *msg, u32 state, u32 timeout)
{
	u32 status = TaskEnterCritical();

	task->state |= state;		// ��������ڵȴ�ĳ���¼���״̬
	task->waitEvent = event;	// ��������ȴ����¼��ṹ
	task->eventMsg = msg;		// ��������ȴ��¼�����Ϣ�洢λ�� 
								// ����ʱ����Ҫ������Ϣ��������Ҫ������
	task->waitEventResult = tErrorNoError;	// ����¼��ĵȴ����

	TaskSchedUnRdy(task);		// ������Ӿ����������Ƴ�

	// ��������뵽�ȴ�������
	ListAddLast(&event->waitList, &task->linkNode);

	// ������������ó�ʱ����ͬʱ���뵽��ʱ������
	// ��ʱ�䵽��ʱ������ʱ������Ƹ����������ʱ�б����Ƴ���ͬʱ���¼��б����Ƴ�
	if (timeout) {
		TimeTaskWait(task, timeout);
	}

	TaskExitCritical(status);
}

/*
** Descriptions         :   ���¼����ƿ��л����׸��ȴ�������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   �׸��ȴ����������û������ȴ����򷵻�0
*/
tTask *EventWakeUp(tEvent *event, void *msg, u32 result)
{
	tNode *node;
	tTask *task;
	
	u32 status = TaskEnterCritical();

	// ȡ���ȴ������еĵ�һ�����
	if ((node = ListRemoveFirst(&event->waitList)) != (tNode *)0) {
		// ת��Ϊ��Ӧ������ṹ
		task = (tTask *)NodeParent(node, tTask, linkNode);

		// �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->waitEventResult = result;
		task->state &= ~TINYOS_TASK_WAIT_MASK;

		// ���������˳�ʱ�ȴ����������£��������ʱ�������Ƴ�
		if (task->delayTicks != 0) {
			TimeTaskWakeUp(task);
		}

		// ����������������
		TaskSchedRdy(task);
	}

	TaskExitCritical(status);

	return task;
}

/*
** Function name        :   tEventRemoveTask
** Descriptions         :   ���������ȴ�������ǿ���Ƴ�
** parameters           :   task ���Ƴ�������
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ��
*/
void EventRemoveTask(tTask *task, void *msg, u32 result)
{
	u32 status = TaskEnterCritical();

	// ����������ڵĵȴ��������Ƴ�
	// ע�⣬����û�м��waitEvent�Ƿ�Ϊ�ա���Ȼ�Ǵ��¼����Ƴ�����ô��Ϊ�Ͳ�����Ϊ��
	ListRemove(&task->waitEvent->waitList, &task->linkNode);

	// �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;
	task->waitEventResult = result;
	task->state &= ~TINYOS_TASK_WAIT_MASK;

	TaskExitCritical(status);
}

/*
** Function name        :   tEventRemoveAll
** Descriptions         :   ������еȴ��е����񣬽��¼����͸���������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ���ѵ���������
*/
u32 EventRemoveAll(tEvent *event, void *msg, u32 result)
{
	tNode *node;
	u32 count;

	tTask *task;

	u32 status = TaskEnterCritical();

	// ��ȡ�ȴ��е���������
	count = ListCount(&event->waitList);

	// �������еȴ��е�����
	while ((node = ListRemoveFirst(&event->waitList)) != (tNode *)0) {
		task = (tTask *)NodeParent(node, tTask, linkNode);

		// �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->waitEventResult = result;
		task->state &= ~TINYOS_TASK_WAIT_MASK;

		// ������������˳�ʱ�ȴ����������ʱ�������Ƴ�
		if (task->delayTicks != 0) {
			TimeTaskWakeUp(task);
		}

		// ����������������
		TaskSchedRdy(task);
	}

	TaskExitCritical(status);

	return count;
}

/*
** Function name        :   tEventWaitCount
** Descriptions         :   �¼����ƿ��еȴ�����������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ���ѵ���������
*/
u32 EventWaitCount(tEvent *event)
{
	u32 count = 0;

	u32 status = TaskEnterCritical();

	count = ListCount(&event->waitList);

	TaskExitCritical(status);

	return count;
}

