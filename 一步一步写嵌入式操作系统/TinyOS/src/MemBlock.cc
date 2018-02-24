#include "Memblock.h"
#include "tinyOS.h"

/*
 * Function name        :   tMemBlockInit
 * Descriptions         :   ��ʼ���洢���ƿ�
 * parameters           :   memBlock �ȴ���ʼ���Ĵ洢���ƿ�
 * parameters           :   memStart �洢������ʼ��ַ
 * parameters           :   blockSize ÿ����Ĵ�С
 * parameters           :   blockCnt �ܵĿ�����
 * Returned value       :   ���ѵ���������
 */
void MemBlockInit(tMemBlock *memBlock, u8 *memStart, u32 blockSize, u32 blockCnt)
{
	u8 *memBlockStart = (u8 *)memStart;
	u8 *memBlcokEnd = memBlockStart + blockSize * blockCnt;

	if (blockSize < sizeof(tNode)) {
		return;
	}

	EventInit(&memBlock->event, tEventTypeMemBlock);

	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;

	ListInit(&memBlock->blockList);
	while (memBlockStart < memBlcokEnd) {
		NodeInit((tNode *)memBlockStart);
		ListAddLast(&memBlock->blockList, (tNode *)memBlockStart);

		memBlockStart += blockSize;
	}
}
/*
** Function name        :   tMemBlockWait
** Descriptions         :   �ȴ��洢��
** parameters           :   memBlock �ȴ��Ĵ洢��
** parameters			:   mem �洢��洢�ĵ�ַ
** parameters           :   waitTicks ��û�д洢��ʱ���ȴ���ticks����Ϊ0ʱ��ʾ��Զ�ȴ�
** Returned value       :   �ȴ����,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
*/
u32 MemBlockWait(tMemBlock *memBlock, u8 **mem, u32 waitTicks)
{
	u32 status = TaskEnterCritical();

	// ���ȼ���Ƿ��п��еĴ洢��
	if (ListCount(&memBlock->blockList) > 0) {
		// ����еĻ���ȡ��һ��
		*mem = (u8 *)ListRemoveFirst(&memBlock->blockList);
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		// ������������¼�������
		EventWait(&memBlock->event, currentTask, (void *)0, tEventTypeMemBlock, waitTicks);
		TaskExitCritical(status);

		// �����ִ��һ���¼����ȣ��Ա����л�����������
		TaskSched();

		// ���л�����ʱ����tTask��ȡ����õ���Ϣ
		*mem = currentTask->eventMsg;

		// ȡ���ȴ����
		return currentTask->waitEventResult;
	}
}

/*
** Function name        :   tMemBlockNoWaitGet
** Descriptions         :   ��ȡ�洢�飬���û�д洢�飬�������˻�
** parameters           :   memBlock �ȴ��Ĵ洢��
** parameters			:   mem �洢��洢�ĵ�ַ
** Returned value       :   ��ȡ���, tErrorResourceUnavaliable.tErrorNoError
*/
u32 MemBlockNoWaitGet(tMemBlock *memBlock, void **mem)
{
	u32 status = TaskEnterCritical();

	// ���ȼ���Ƿ��п��еĴ洢��
	if (ListCount(&memBlock->blockList) > 0) {
		// ����еĻ���ȡ��һ��
		*mem = (u8 *)ListRemoveFirst(&memBlock->blockList);
		TaskExitCritical(status);
		return tErrorNoError;
	} else {
		// ���򷵻���Դ������
		TaskExitCritical(status);
		return tErrorResourceUnavalible;
	}
}

/*
** Function name        :   tMemBlockNotify
** Descriptions         :   ֪ͨ�洢����ã����ѵȴ������е�һ�����񣬻��߽��洢����������
** parameters           :   memBlock �������ź���
** Returned value       :   ��
*/
void MemBlockNotify(tMemBlock *memBlock, u8 *mem)
{
	u32 status = TaskEnterCritical();

	// ����Ƿ�������ȴ�
	if (EventWaitCount(&memBlock->event) > 0) {
		// ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
		tTask *task = EventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);

		// ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
		if (task->prio < currentTask->prio) {
			TaskSched();
		}
	} else {
		// ���û������ȴ��Ļ������洢����뵽������
		ListAddLast(&memBlock->blockList, (tNode *)mem);
	}

	TaskExitCritical(status);
}

