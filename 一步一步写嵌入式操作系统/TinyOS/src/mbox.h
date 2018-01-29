#ifndef MBOX_H
#define MBOX_H

#define tMboxSendNormal		0x00
#define tMboxSendFront		0x01

typedef struct _tMbox
{
	tEvent event;
	u32 count;
	u32 read;
	u32 write;
	u32 maxCount;
	void **msgBuffer;
} tMbox;

typedef struct _tMboxInfo
{
	u32 count;
	u32 maxCount;
	u32 taskCount;
} tMboxInfo;

void MboxInit(tMbox *mbox, void **msgBuffer, u32 maxCount);
u32 MboxWait(tMbox *mbox, void **msg, u32 waitTicks);
u32 MboxNoWaitGet(tMbox *mbox, void **msg);
u32 MboxNotify(tMbox *mbox, void *msg, u32 notifyOption);
void MboxFlush(tMbox *mbox);
u32 MboxDestroy(tMbox *mbox);
void MboxGetInfo(tMbox *mbox, tMboxInfo *info);

#endif

