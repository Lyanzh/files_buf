#include "tinyOS.h"

void taskDelay(u32 delay)
{
	u32 status = TaskEnterCritical();

	// 设置延时值，插入延时队列
	TimeTaskWait(currentTask, delay);

	// 将任务从就绪表中移除
	TaskSchedUnRdy(currentTask);

	TaskSched();
	
	TaskExitCritical(status);
}

