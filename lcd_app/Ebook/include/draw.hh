#ifndef __DRAW__
#define __DRAW__

#include "encoding_manager.h"
#include "fonts_manager.h"
#include "disp_manager.h"

extern PT_Encoding_Opr g_ptEncodingOprForFile;
extern PT_DispDev g_ptDispOpr;

extern unsigned char *g_pucFileMemStart;
extern unsigned char *g_pucFileMemEnd;
extern unsigned char *g_pucLcdFirstPosAtFile;

extern int Open_Text_File(char *pcPathName);
extern int Select_And_Init_Display(char *pcName);
extern int Set_Text_Detail(char *pcHZKFile, char *pcFreetypeFile, unsigned int dwFontSize);
extern int Show_Pre_Page(void);
extern int Show_Next_Page(void);
#endif

