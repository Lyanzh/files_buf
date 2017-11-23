#ifndef _PAGE_MEM_H
#define _PAGE_MEM_H

#define MAINPAGE_ID		0
#define BROWSEPAGE_ID	1
#define AUTOPAGE_ID		2

typedef enum Page_Mem_State
{
	PAGE_MEM_FREE,	/* no data  */
	PAGE_MEM_PACKED,/* has data */
	PAGE_MEM_BUSY,	/* showing  */
} E_Page_Mem_State, *PE_Page_Mem_State;

typedef struct Page_Mem_Desc
{
	int iID;
	unsigned int dwMemSize;
	char *pcMem;
	E_Page_Mem_State State;
	struct Page_Mem_Desc *ptNext;
} T_Page_Mem, *PT_Page_Mem;

typedef PT_Page_Mem PT_Page_Mem_List;

extern int Page_Mem_Prepare(int iNum);

#endif