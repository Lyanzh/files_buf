#include "global.h"
#include "ARMCM3.h"
#include "tinyOS.h"

tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

void idleTaskEntry(void *param)
{
	while (1);
}

tTask tTask1;
tTask tTask2;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

int task1Flag;
void task1Entry(void *param)
{
	SetSysTickPeriod(10);
	while (1) {
		TaskSchedDisable();
		task1Flag = 1;
		taskDelay(10);
		task1Flag = 0;
		taskDelay(10);
		//TaskSched();
		TaskSchedEnable();
	}
}

int task2Flag;
void task2Entry(void *param)
{
	while (1) {
		task2Flag = 1;
		taskDelay(10);
		task2Flag = 0;
		taskDelay(10);
		//TaskSched();
	}
}

int main(void)
{
	TaskSchedInit();
	
	TaskInit(&tTask1, task1Entry, (void *)0, 0, &task1Env[1024]);
	TaskInit(&tTask2, task2Entry, (void *)0, 1, &task2Env[1024]);
	TaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRIO_COUNT - 1, &idleTaskEnv[1024]);
	
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	idleTask = &tTaskIdle;

	nextTask = &tTask1;

	TaskRunFirst();
	
	return 0;
}

