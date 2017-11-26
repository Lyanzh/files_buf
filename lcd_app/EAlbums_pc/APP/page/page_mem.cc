#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "memwatch.h"

static pthread_mutex_t g_tPageMemListMutex = PTHREAD_MUTEX_INITIALIZER;
PT_Page_Mem_List g_ptPageMemListHead;/* 空头部，方便删除操作 */

static void Page_Mem_List_Add(PT_Page_Mem ptPageMemNew)
{
	PT_Page_Mem ptPageMemTmp;
	pthread_mutex_lock(&g_tPageMemListMutex);
	if (!g_ptPageMemListHead) {
		g_ptPageMemListHead = (PT_Page_Mem)malloc(sizeof(T_Page_Mem));/* 创建空头部 */
		g_ptPageMemListHead->ptNext = ptPageMemNew;
	} else {
		ptPageMemTmp = g_ptPageMemListHead->ptNext;
		while (ptPageMemTmp->ptNext) {
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}
		ptPageMemTmp->ptNext = ptPageMemNew;
	}
	ptPageMemNew->ptNext = NULL;
	pthread_mutex_unlock(&g_tPageMemListMutex);
}

void Page_Mem_List_Del(int iPageID)
{
	PT_Page_Mem ptPageMemPre;
	PT_Page_Mem ptPageMemTmp;
	pthread_mutex_lock(&g_tPageMemListMutex);
	ptPageMemPre = g_ptPageMemListHead;
	ptPageMemTmp = g_ptPageMemListHead->ptNext;
	if (!ptPageMemTmp) {
		return;
	} else {
		while (ptPageMemTmp->iPageID != iPageID) {
			ptPageMemPre = ptPageMemTmp;
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}
		ptPageMemPre->ptNext = ptPageMemTmp->ptNext;
		free(ptPageMemTmp->pcMem);
		free(ptPageMemTmp);
	}
	pthread_mutex_unlock(&g_tPageMemListMutex);
}

void Page_Grop_Mem_List_Del(int iPageGropID)
{
	PT_Page_Mem ptPageMemPre;
	PT_Page_Mem ptPageMemTmp;
	pthread_mutex_lock(&g_tPageMemListMutex);
	ptPageMemPre = g_ptPageMemListHead;
	ptPageMemTmp = g_ptPageMemListHead->ptNext;
	if (!ptPageMemTmp) {
		return;
	} else {
		while (ptPageMemTmp) {
			if ((ptPageMemTmp->iPageID & PAGE_GROUP_MASK) == iPageGropID) {
				/* delete */
				ptPageMemPre->ptNext = ptPageMemTmp->ptNext;
				free(ptPageMemTmp->pcMem);
				free(ptPageMemTmp);
			} else {
				ptPageMemPre = ptPageMemTmp;
				ptPageMemTmp = ptPageMemTmp->ptNext;
			}
		}
	}
	pthread_mutex_unlock(&g_tPageMemListMutex);
}

PT_Page_Mem Page_Mem_Alloc(int iPageID)
{
	PT_Page_Mem ptPageMemNew;
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
		free(ptPageMemNew);
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
	if (!g_ptPageMemListHead->ptNext) {
		return NULL;
	} else {
		ptPageMemTmp = g_ptPageMemListHead->ptNext;
		while (ptPageMemTmp->iPageID != iPageID) {
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}
		return ptPageMemTmp;
	}
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

