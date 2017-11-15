#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>

#define BROWSEMODE_X(w)	((Selected_Display()->tDevAttr.dwXres - w) / 2)
#define BROWSEMODE_Y(h)	((Selected_Display()->tDevAttr.dwYres - h) / 3)

#define CONTINUEMODE_X(w)	((Selected_Display()->tDevAttr.dwXres - w) / 2)
#define CONTINUEMODE_Y(h)	((Selected_Display()->tDevAttr.dwYres - h) / 2)

#define SETTINGMODE_X(w)	((Selected_Display()->tDevAttr.dwXres - w) / 2)
#define SETTINGMODE_Y(h)	((Selected_Display()->tDevAttr.dwYres - h) / 3 * 2)

T_PicRegion tBrowseModeSrc;
T_PicRegion tContinueModeSrc;
T_PicRegion tSettingSrc;
T_PicRegion tBrowseModeDst;
T_PicRegion tContinueModeDst;
T_PicRegion tSettingDst;

static void Main_Page_Prepare(void)
{
	//Pic_Zoom(PT_PicRegion ptDstPicReg, PT_PicRegion ptSrcPicReg);
}

static void Main_Page_Run(void)
{
	Get_Format_Opr("bmp")->Get_Pic_Region("icon/browse_mode.bmp", &tBrowseModeSrc);
	Pic_Zoom(&tBrowseModeDst, &tBrowseModeSrc);
	Fb_Lcd_Show_Pic(BROWSEMODE_X(tBrowseModeDst.dwWidth), BROWSEMODE_Y(tBrowseModeDst.dwHeight), &tBrowseModeDst);
	//free(tBrowseModeSrc.pcData);
	//free(tBrowseModeDst.pcData);
	
	Get_Format_Opr("bmp")->Get_Pic_Region("icon/continue_mode.bmp", &tContinueModeSrc);
	Pic_Zoom(&tContinueModeDst, &tContinueModeSrc);
	Fb_Lcd_Show_Pic(CONTINUEMODE_X(tContinueModeDst.dwWidth), CONTINUEMODE_Y(tContinueModeDst.dwHeight), &tContinueModeDst);
	//free(tContinueModeSrc.pcData);
	//free(tContinueModeDst.pcData);
	
	Get_Format_Opr("bmp")->Get_Pic_Region("icon/setting.bmp", &tSettingSrc);
	Pic_Zoom(&tSettingDst, &tSettingSrc);
	Fb_Lcd_Show_Pic(SETTINGMODE_X(tSettingDst.dwWidth), SETTINGMODE_Y(tSettingDst.dwHeight), &tSettingDst);
	//free(tSettingSrc.pcData);
	//free(tSettingDst.pcData);
}

static void Main_Page_Get_Input_Event(void)
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

static T_Page_Opr g_tMainPageOpr = {
	.c_pcName   = "mainpage",
	.Prepare    = Main_Page_Prepare,
	.Run		= Main_Page_Run,
	.Get_Input_Event = Main_Page_Get_Input_Event,
};

int Main_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tMainPageOpr);
}

