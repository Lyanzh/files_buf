#ifndef _PAGE_MEM_H
#define _PAGE_MEM_H

#define MAINPAGE_GROUP			0x0100
#define BROWSEPAGE_GROUP		0x0200
#define AUTOPAGE_GROUP			0x0300
#define SETTINGPAGE_GROUP		0x0400
#define PAGE_GROUP_MASK			0xFF00

#define MAINPAGE_MAIN			(MAINPAGE_GROUP    | 0x01)
#define BROWSEPAGE_MAIN			(BROWSEPAGE_GROUP  | 0x01)
#define BROWSEPAGE_LARGER		(BROWSEPAGE_GROUP  | 0x02)
#define BROWSEPAGE_SMALLER		(BROWSEPAGE_GROUP  | 0x03)
#define BROWSEPAGE_NEXT			(BROWSEPAGE_GROUP  | 0x04)
#define BROWSEPAGE_PRE			(BROWSEPAGE_GROUP  | 0x05)
#define AUTOPAGE_MAIN			(AUTOPAGE_GROUP    | 0x01)
#define AUTOPAGE_NEXT			(AUTOPAGE_GROUP    | 0x02)
#define SETTINGPAGE_MAIN		(SETTINGPAGE_GROUP | 0x01)

typedef enum Page_Mem_State
{
	PAGE_MEM_FREE,	/* no data  */
	PAGE_MEM_PACKED,/* has data */
} E_Page_Mem_State, *PE_Page_Mem_State;

typedef struct Page_Mem_Desc
{
	int iPageID;
	unsigned int dwMemSize;
	char *pcMem;
	E_Page_Mem_State State;
	struct Page_Mem_Desc *ptPre;
	struct Page_Mem_Desc *ptNext;
} T_Page_Mem, *PT_Page_Mem;

typedef PT_Page_Mem PT_Page_Mem_List;

extern int Page_Mem_Prepare(int iNum);
extern PT_Page_Mem Page_Mem_Alloc(int iPageID);
extern void Page_Mem_List_Del(int iPageID);
extern void Page_Grop_Mem_List_Del(int iPageGropID);
extern PT_Page_Mem Page_Mem_Get(int iPageID);
extern void Page_Mem_Init(void);

#endif