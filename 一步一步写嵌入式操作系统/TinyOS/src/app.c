#include "tinyOS.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];

u32 task1Flag;
void task1DestroyFunc(void *param)
{
	task1Flag = 0;
}

void task1Entry(void *param)
{
	SetSysTickPeriod(10);

	TaskSetCleanCallFunc(currentTask, task1DestroyFunc, (void *)0);
	while (1) {
		//TaskSchedDisable();
		task1Flag = 1;
		//taskDelay(10);
		TaskSuspend(currentTask);
		task1Flag = 0;
		//taskDelay(10);
		TaskSuspend(currentTask);
		//TaskSchedEnable();
	}
}

u32 task2Flag;
void task2Entry(void *param)
{
	u8 task1Deleted = 0;
	while (1) {
		task2Flag = 1;
		taskDelay(10);
		task2Flag = 0;
		taskDelay(10);

		if (!task1Deleted) {
			TaskForceDelete(&tTask1);
			task1Deleted = 1;
		}
	}
}

u32 task3Flag;
void task3Entry(void *param)
{
	while (1) {
		if (TaskIsRequestedDelete()) {
			task3Flag = 0;
			TaskDeleteSelf();
		}
		task3Flag = 1;
		taskDelay(10);
		task3Flag = 0;
		taskDelay(10);
	}
}

u32 task4Flag;
void task4Entry(void *param)
{
	u8 task3Deleted = 0;
	while (1) {
		task4Flag = 1;
		taskDelay(10);
		task4Flag = 0;
		taskDelay(10);

		if (!task3Deleted) {
			TaskRequestDelete(&tTask3);
			task3Deleted = 1;
		}
	}
}


void InitApp(void)
{
	TaskInit(&tTask1, task1Entry, (void *)0, 0, &task1Env[1024]);
	TaskInit(&tTask2, task2Entry, (void *)0, 1, &task2Env[1024]);
	TaskInit(&tTask3, task3Entry, (void *)0, 1, &task3Env[1024]);
	TaskInit(&tTask4, task4Entry, (void *)0, 1, &task4Env[1024]);
}

