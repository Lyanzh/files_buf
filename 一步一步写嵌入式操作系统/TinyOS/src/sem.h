#ifndef SEM_H
#define SEM_H

#include "event.h"

typedef struct _tSem
{
	tEvent event;
	u32 count;
	u32 maxCount;
} tSem;

typedef struct _tSemInfo
{
	// ��ǰ�ź����ļ���
	u32 count;

	// �ź��������������
	u32 maxCount;

	// ��ǰ�ȴ����������
	u32 taskCount;
} tSemInfo;

void SemInit(tSem *sem, u32 startCount, u32 maxCount);
u32 SemWait(tSem *sem, u32 waitTicks);
void SemNotify(tSem *sem);
u32 SemNoWaitGet(tSem *sem);
u32 SemDestroy(tSem *sem);
void SemGetInfo(tSem *sem, tSemInfo *info);

#endif

