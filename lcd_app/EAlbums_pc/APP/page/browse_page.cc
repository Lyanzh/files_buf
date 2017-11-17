#include "page_manager.h"
#include <stdio.h>
#include <stdlib.h>

#define ZOOMOUT_X(w)				(0)
#define ZOOMOUT_Y(h)				(0)

#define ZOOMIN_X(w)					(1 * w)
#define ZOOMIN_Y(h)					(0)

#define CONTINUE_MODE_SMALL_X(w)	(2 * w)
#define CONTINUE_MODE_SMALL_Y(h)	(0)

#define PRE_PIC_X(w)				(3 * w)
#define PRE_PIC_Y(h)				(0)

#define NEXT_PIC_X(w)				(4 * w)
#define NEXT_PIC_Y(h)				(0)

T_IconInfo t_BrowsePageIcon[] = 
{
	{"icon/zoomout.bmp", 0, 0},
	{"icon/zoomin.bmp", 0, 0},
	{"icon/continue_mode_small.bmp", 0, 0},
	{"icon/pre_pic.bmp", 0, 0},
	{"icon/next_pic.bmp", 0, 0},
};

T_PicRegion tPicRegSrc;
T_PicRegion tPicRegDst;

static void Browse_Page_Prepare(void)
{
    DIR *dir;
    char basePath[100];

    //get the current absoulte path
    memset(basePath, '\0', sizeof(basePath));
    getcwd(basePath, 99);
    printf("the current dir is : %s\n", basePath);

    //get the file list
    Read_File_List(basePath);

    
}

static void Browse_Page_Run(void)
{
	int i;
	int iIconNum;

	iIconNum = sizeof(t_BrowsePageIcon) / sizeof(T_IconInfo);
	for (i = 0; i < iIconNum; i++) {
		Get_Format_Opr("bmp")->Get_Pic_Region(t_BrowsePageIcon[i].pcName, &tPicRegSrc);
		Pic_Zoom(&tPicRegDst, &tPicRegSrc);
		Fb_Lcd_Show_Pic(tPicRegDst.dwWidth * i, t_BrowsePageIcon[i].iY, &tPicRegDst);
	}

	Get_Format_Opr("jpeg")->Get_Pic_Region("1.jpg", &tPicRegSrc);
	//Pic_Zoom(&tPicRegDst, &tPicRegSrc);
	Fb_Lcd_Show_Pic(0, tPicRegDst.dwHeight, &tPicRegSrc);
	//free(tBrowseModeSrc.pcData);
	//free(tBrowseModeDst.pcData);
}

static void Browse_Page_Get_Input_Event(void)
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

static T_Page_Opr g_tBrowsePageOpr = {
	.c_pcName   = "browsepage",
	.Prepare    = Browse_Page_Prepare,
	.Run		= Browse_Page_Run,
	.Get_Input_Event = Browse_Page_Get_Input_Event,
};

int Browse_Page_Init(void)
{
	return Page_Opr_Regisiter(&g_tBrowsePageOpr);
}

