#include "page_manager.h"

static void Main_Page_Run(void)
{
	
}

static T_Page_Opr g_tMainPageOpr = {
	.c_pcName   = "mainpage",
	.Run		= Main_Page_Run,
	.Prepare    = Main_Page_Prepare,
	.Get_Input_Event = Main_Page_Get_Input_Event,
};

int Main_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tMainPageOpr);
}

