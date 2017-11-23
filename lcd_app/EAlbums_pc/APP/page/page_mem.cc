#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memwatch.h"

PT_Page_Mem_List g_ptPageMemListHead;

static void Page_Mem_List_Add(PT_Page_Mem ptPageMemNew)
{
	PT_Page_Mem ptPageMemTmp;
	if (!g_ptPageMemListHead) {
		g_ptPageMemListHead = ptPageMemNew;
	} else {
		ptPageMemTmp = g_ptPageMemListHead;
		while (ptPageMemTmp->ptNext) {
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}
		ptPageMemNew->iID = ptPageMemTmp->iID + 1;
		ptPageMemTmp->ptNext = ptPageMemNew;
	}
	ptPageMemNew->ptNext = NULL;
}

static PT_Page_Mem Page_Mem_Alloc(void)
{
	PT_Page_Mem ptPageMemNew;
	ptPageMemNew = (PT_Page_Mem)malloc(sizeof(T_Page_Mem));
	if (!ptPageMemNew) {
		printf("Error:malloc ptPageMemNew error\n");
		return NULL;
	}

	ptPageMemNew->iID = 0;
	ptPageMemNew->dwMemSize = Selected_Display()->dwScreenSize;
	ptPageMemNew->pcMem = (char *)malloc(ptPageMemNew->dwMemSize);
	if (!ptPageMemNew->pcMem) {
		printf("Error:malloc pcMem error\n");
		free(ptPageMemNew);
		return NULL;
	}

	ptPageMemNew->State = PAGE_MEM_FREE;
	return ptPageMemNew;
}

PT_Page_Mem Page_Mem_Get(int iID)
{
	PT_Page_Mem ptPageMemTmp;
	if (!g_ptPageMemListHead) {
		return NULL;
	} else {
		ptPageMemTmp = g_ptPageMemListHead;
		while (ptPageMemTmp->iID != iID) {
			ptPageMemTmp = ptPageMemTmp->ptNext;
		}
		return ptPageMemTmp;
	}
}

PT_Page_Mem Page_Mem_Free_Find(void)
{
	PT_Page_Mem ptPageMemTmp;
	if (!g_ptPageMemListHead) {
		return NULL;
	} else {
		ptPageMemTmp = g_ptPageMemListHead;
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

