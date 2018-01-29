#include "tinyOS.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];

tMbox mbox1;
tMbox mbox2;
void *mbox1MsgBuffer[20];
void *mbox2MsgBuffer[20];

u32 msg[20];

u32 task1Flag;
void task1Entry(void *param)
{
	void *msg;
	
	SetSysTickPeriod(10);

	MboxInit(&mbox1, (void *)mbox1MsgBuffer, 20);

	MboxWait(&mbox1, &msg, 0);
	while (1) {	
		task1Flag = 1;
		taskDelay(10);
		task1Flag = 0;
		taskDelay(10);
	}
}

u32 task2Flag;
void task2Entry(void *param)
{
	u32 isMboxDestroyed = 0;
	while (1) {
		task2Flag = 1;
		taskDelay(10);
		task2Flag = 0;
		taskDelay(10);

		if (!isMboxDestroyed) {
			MboxDestroy(&mbox1);
		}
	}
}

u32 task3Flag;
void task3Entry(void *param)
{
	u32 i;
	MboxInit(&mbox2, (void *)mbox2MsgBuffer, 20);
	while (1) {
		for (i = 0; i < 20; i++) {
			msg[i] = i;
			MboxNotify(&mbox2, (void *)&msg[i], tMboxSendFront);
		}
		taskDelay(100);
		task3Flag = 1;
		taskDelay(1);
		task3Flag = 0;
		taskDelay(1);
	}
}

u32 task4Flag;
void task4Entry(void *param)
{
	u32 err;
	void *msg;
	u32 value;
	while (1) {
		err = MboxWait(&mbox2, &msg, 0);
		if (err == tErrorNoError) {
			value = *(u32 *)msg;
			task4Flag = value;
			MboxFlush(&mbox2);
			taskDelay(1);
		}
		//task4Flag = 1;
		//taskDelay(1);
		//task4Flag = 0;
		//taskDelay(1);
	}
}

void InitApp(void)
{
	TaskInit(&tTask1, task1Entry, (void *)0, 0, &task1Env[1024]);
	TaskInit(&tTask2, task2Entry, (void *)0, 1, &task2Env[1024]);
	TaskInit(&tTask3, task3Entry, (void *)0, 0, &task3Env[1024]);
	TaskInit(&tTask4, task4Entry, (void *)0, 1, &task4Env[1024]);
}

