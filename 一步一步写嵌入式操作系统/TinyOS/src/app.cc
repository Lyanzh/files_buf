#include "tinyOS.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];

tEvent eventWaitNormal;

u32 task1Flag;
void task1Entry(void *param)
{
	u32 wakeUpCount;
	u32 count;
	
	SetSysTickPeriod(10);

	EventInit(&eventWaitNormal, tEventTypeUnknown);

	while (1) {
		count = EventWaitCount(&eventWaitNormal);
		wakeUpCount = EventRemoveAll(&eventWaitNormal, (void *)0, 0);
		if (wakeUpCount > 0) {
			TaskSched();

			count = EventWaitCount(&eventWaitNormal);
		}
		
		task1Flag = 1;
		taskDelay(10);
		task1Flag = 0;
		taskDelay(10);
	}
}

u32 task2Flag;
void task2Entry(void *param)
{
	while (1) {
		EventWait(&eventWaitNormal, currentTask, (void *)0, 0, 0);
		TaskSched();

		task2Flag = 1;
		taskDelay(10);
		task2Flag = 0;
		taskDelay(10);
	}
}

u32 task3Flag;
void task3Entry(void *param)
{
	while (1) {
		EventWait(&eventWaitNormal, currentTask, (void *)0, 0, 0);
		TaskSched();

		task3Flag = 1;
		taskDelay(10);
		task3Flag = 0;
		taskDelay(10);
	}
}

u32 task4Flag;
void task4Entry(void *param)
{
	while (1) {
		EventWait(&eventWaitNormal, currentTask, (void *)0, 0, 0);
		TaskSched();
		
		task4Flag = 1;
		taskDelay(10);
		task4Flag = 0;
		taskDelay(10);
	}
}


void InitApp(void)
{
	TaskInit(&tTask1, task1Entry, (void *)0, 0, &task1Env[1024]);
	TaskInit(&tTask2, task2Entry, (void *)0, 1, &task2Env[1024]);
	TaskInit(&tTask3, task3Entry, (void *)0, 1, &task3Env[1024]);
	TaskInit(&tTask4, task4Entry, (void *)0, 1, &task4Env[1024]);
}

