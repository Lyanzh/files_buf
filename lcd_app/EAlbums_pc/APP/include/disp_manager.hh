#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

typedef struct Dev_Attr
{
	unsigned int dwXres;			/* visible resolution		*/
	unsigned int dwYres;
	unsigned int dwBitsPerPixel;
}T_Dev_Attr, *PT_Dev_Attr;

typedef struct Disp_Opr
{
	const char *pcName;
	T_Dev_Attr tDevAttr;
	unsigned int dwScreenSize;
	char *pcMem;
	int (*Init)(void);
	int (*Clean_Screen)(void);
	void (*Put_Pixel)(int, int, unsigned int);
	int (*Dev_Remove)(void);
	struct Disp_Opr *ptNext;
}T_Disp_Opr, *PT_Disp_Opr;

int Fb_Dev_Init(void);
int PC_Disp_Dev_Init(void);

extern int Disp_Opr_Regisiter(PT_Disp_Opr ptDispOpr);
extern void Show_Disp_Opr(void);
extern PT_Disp_Opr Get_Disp_Opr(char *pcName);
extern int Disp_Opr_Init(void);
extern int Select_And_Init_Display(char *pcName);
extern PT_Disp_Opr Selected_Display(void);

#endif
