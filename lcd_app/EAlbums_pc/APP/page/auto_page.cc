#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>

T_PicRegion tPicRegSrc;
//T_PicRegion tPicRegDst;

static void Auto_Page_Prepare(void)
{
}

static void Auto_Page_Run(void)
{
	Get_Format_Opr("jpeg")->Get_Pic_Region("1.jpg", &tPicRegSrc);
	//Pic_Zoom(&tPicRegDst, &tPicRegSrc, 1);
	Fb_Lcd_Show_Pic(0, 0, &tPicRegSrc);
	//free(tBrowseModeSrc.pcData);
	//free(tBrowseModeDst.pcData);
}

static void Auto_Page_Get_Input_Event(void)
{
	T_Input_Event tInputEvent;
	Input_Get_Key(&tInputEvent);
	if (tInputEvent.cCode == 'b') {
		printf("\nshow browse page.\n");
		//Socket_Send("show next page.\n");
	} else if (tInputEvent.cCode == 'c') {
		printf("\nshow continue mode page.\n");
	} else if (tInputEvent.cCode == 's') {
		printf("\nshow setting page.\n");
	}
}

static T_Page_Opr g_tAutoPageOpr = {
	.c_pcName   = "autopage",
	.Prepare    = Auto_Page_Prepare,
	.Run		= Auto_Page_Run,
	.Get_Input_Event = Auto_Page_Get_Input_Event,
};

int Auto_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tAutoPageOpr);
}

