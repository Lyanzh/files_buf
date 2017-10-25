#ifndef __DISP_MANAGER__
#define __DISP_MANAGER__

typedef struct DevAttr
{
	unsigned int dwXres;			/* visible resolution		*/
	unsigned int dwYres;
	unsigned int dwBitsPerPixel;
}T_DevAttr, *PT_DevAttr;

typedef struct DispDevice
{
	const char * c_pDevName;
	T_DevAttr tDevAttr;
	int (*Dev_Init)(void);
	int (*Clean_Screen)(void);
	void (*Put_Pixel)(int, int, unsigned int);
	int (*Dev_Remove)(void);
	struct DispDevice *ptNextDev;
}T_DispDev, *PT_DispDev;

int Fb_Dev_Init(void);

extern PT_DispDev g_ptDispOprSelected;

extern int Disp_Dev_Regisiter(PT_DispDev ptDispDev);
extern void Show_Disp_Opr(void);
extern PT_DispDev Get_Disp_Opr(char *pcName);
extern int Disp_Opr_Init(void);
extern int Select_And_Init_Display(char *pcName);

#endif
