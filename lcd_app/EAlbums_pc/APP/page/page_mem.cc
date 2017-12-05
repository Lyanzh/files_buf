#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//#include "memwatch.h"

static pthread_mutex_t g_tPageMemListMutex = PTHREAD_MUTEX_INITIALIZER;
static PT_Page_Mem_List g_ptPageMemListHead;/* 空表头，方便插入删除操作 */

static void Page_Mem_List_Add(PT_Page_Mem ptPageMemNew)
{
	PT_Page_Mem ptPageMemTmp;
	pthread_mutex_lock(&g_tPageMemListMutex);
	if (!g_ptPageMemListHead) {
		printf("Error:please initialize g_ptPageMemListHead first\n");
	} else {
		ptPageMemTmp = g_ptPageMemListHead;
		while (ptPageMemTmp->ptNext) {
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}
		ptPageMemTmp->ptNext = ptPageMemNew;
		ptPageMemNew->ptPre = ptPageMemTmp;
		ptPageMemNew->ptNext = NULL;
	}
	pthread_mutex_unlock(&g_tPageMemListMutex);
}

void Page_Mem_List_Del(int iPageID)
{
	PT_Page_Mem ptPageMemTmp;
	pthread_mutex_lock(&g_tPageMemListMutex);
	ptPageMemTmp = g_ptPageMemListHead->ptNext;
	if (!ptPageMemTmp) {
		/* 只有头部，无实际成员，直接退出 */
		goto unlock;
	} else {
		while (ptPageMemTmp) {
			if (ptPageMemTmp->iPageID == iPageID) {
				/* delete */
				if (!ptPageMemTmp->ptNext) {
					/* 链表尾 */
					ptPageMemTmp->ptPre->ptNext = NULL;
				} else {
					ptPageMemTmp->ptNext->ptPre = ptPageMemTmp->ptPre;
					ptPageMemTmp->ptPre->ptNext = ptPageMemTmp->ptNext;
				}
				Do_Free(ptPageMemTmp->pcMem);
				Do_Free(ptPageMemTmp);
				goto unlock;
			} else {
				ptPageMemTmp = ptPageMemTmp->ptNext;
			}
		}
	}
unlock:
	pthread_mutex_unlock(&g_tPageMemListMutex);
}

void Page_Grop_Mem_List_Del(int iPageGropID)
{
	PT_Page_Mem ptPageMemTmp;
	PT_Page_Mem ptPageMemTmpFree;
	pthread_mutex_lock(&g_tPageMemListMutex);
	ptPageMemTmp = g_ptPageMemListHead->ptNext;
	if (!ptPageMemTmp) {
		/* 只有头部，无实际成员，直接退出 */
		goto unlock;
	} else {
		while (ptPageMemTmp) {
			if ((ptPageMemTmp->iPageID & PAGE_GROUP_MASK) == iPageGropID) {
				/* delete */
				if (!ptPageMemTmp->ptNext) {
					/* 链表尾 */
					ptPageMemTmp->ptPre->ptNext = NULL;
					Do_Free(ptPageMemTmp->pcMem);
					Do_Free(ptPageMemTmp);
					goto unlock;
				} else {
					ptPageMemTmp->ptNext->ptPre = ptPageMemTmp->ptPre;
					ptPageMemTmp->ptPre->ptNext = ptPageMemTmp->ptNext;

					ptPageMemTmpFree = ptPageMemTmp;
					
					/* 指向下一个 */
					ptPageMemTmp = ptPageMemTmp->ptNext;

					/* free */
					Do_Free(ptPageMemTmpFree->pcMem);
					Do_Free(ptPageMemTmpFree);
				}
			} else {
				ptPageMemTmp = ptPageMemTmp->ptNext;
			}
		}
	}
unlock:
	pthread_mutex_unlock(&g_tPageMemListMutex);
}

PT_Page_Mem Page_Mem_Alloc(int iPageID)
{
	PT_Page_Mem ptPageMemNew;
	printf("Page_Mem_Alloc %x\n", iPageID);
	ptPageMemNew = (PT_Page_Mem)malloc(sizeof(T_Page_Mem));
	if (!ptPageMemNew) {
		printf("Error:malloc ptPageMemNew error\n");
		return NULL;
	}

	ptPageMemNew->iPageID = iPageID;
	ptPageMemNew->dwMemSize = Selected_Display()->dwScreenSize;
	ptPageMemNew->pcMem = (char *)malloc(ptPageMemNew->dwMemSize);
	if (!ptPageMemNew->pcMem) {
		printf("Error:malloc pcMem error\n");
		Do_Free(ptPageMemNew);
		return NULL;
	}

	memset(ptPageMemNew->pcMem, 0, ptPageMemNew->dwMemSize);
	ptPageMemNew->State = PAGE_MEM_FREE;
	Page_Mem_List_Add(ptPageMemNew);
	
	return ptPageMemNew;
}

PT_Page_Mem Page_Mem_Get(int iPageID)
{
	PT_Page_Mem ptPageMemTmp;
	printf("Page_Mem_Get %x\n", iPageID);
	if (!g_ptPageMemListHead->ptNext) {
		return NULL;
	} else {
		printf("Page_Mem_Get 2 %x\n", iPageID);
		ptPageMemTmp = g_ptPageMemListHead->ptNext;
		while (ptPageMemTmp) {
			if (ptPageMemTmp->iPageID == iPageID) {
				printf("Page_Mem_Get 3 %x\n", iPageID);
				return ptPageMemTmp;
			}
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}
		return NULL;
	}
}

void Page_Mem_Init(void)
{
	/* 创建空头部 */
	g_ptPageMemListHead = (PT_Page_Mem)malloc(sizeof(T_Page_Mem));
	g_ptPageMemListHead->ptPre = NULL;
	g_ptPageMemListHead->ptNext = NULL;
}

#if 0
PT_Page_Mem Page_Mem_Free_Find(void)
{
	PT_Page_Mem ptPageMemTmp;
	if (!g_ptPageMemListHead->ptNext) {
		return NULL;
	} else {
		ptPageMemTmp = g_ptPageMemListHead->ptNext;
		while (ptPageMemTmp->State != PAGE_MEM_FREE) {
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}

		if (ptPageMemTmp == NULL) {/* 缓存已经用完 */
			
		}
		
		return ptPageMemTmp;
	}
}

int Page_Mem_Prepare(int iNum)
{
	int i;
	PT_Page_Mem ptPageMemNew;
	for (i = 0; i < iNum; i++) {
		ptPageMemNew = Page_Mem_Alloc();
		if (ptPageMemNew) {
			Page_Mem_List_Add(ptPageMemNew);
		} else {
			printf("Error:Page_Mem_Prepare error\n");
			return -1;
		}
	}
	return 0;
}
#endif

