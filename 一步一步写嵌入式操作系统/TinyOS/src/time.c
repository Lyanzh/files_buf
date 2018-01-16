#include "tinyOS.h"

void taskDelay(u32 delay)
{
	u32 status = TaskEnterCritical();

	// ������ʱֵ��������ʱ����
	TimeTaskWait(currentTask, delay);

	// ������Ӿ��������Ƴ�
	TaskSchedUnRdy(currentTask);

	TaskSched();
	
	TaskExitCritical(status);
}

