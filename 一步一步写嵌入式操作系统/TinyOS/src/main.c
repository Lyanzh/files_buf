#include "global.h"
#include "ARMCM3.h"
#include "tinyOS.h"

tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

void idleTaskEntry(void *param)
{
	while (1);
}

int main(void)
{
	TaskSchedInit();

	TaskDelayInit();

	InitApp();
	TaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRIO_COUNT - 1, &idleTaskEnv[1024]);

	nextTask = TaskHighestReady();

	TaskRunFirst();
	
	return 0;
}

