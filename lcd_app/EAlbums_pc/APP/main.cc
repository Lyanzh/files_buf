#include "config.h"
#include "draw.h"
#include "page_mem.h"
//#include "encoding_manager.h"
//#include "fonts_manager.h"
#include "disp_manager.h"
//#include "input_manager.h"
//#include "socket_manager.h"
#include "format_manager.h"
#include "page_manager.h"

#include "memwatch.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	//T_Input_Event tInputEvent;
	//T_PicRegion tPicReg;

	Disp_Opr_Init();
	Format_Opr_Init();
	Page_Opr_Init();
	//Font_Opr_Init();
	//Encoding_Opr_Init();
	Input_Opr_Init();
	//Socket_Opr_Init();

	//Show_Disp_Opr();
	//Show_Font_Opr();
	//Show_Encoding_Opr();
	//Show_Input_Opr();

	Page_Mem_Init();

	Select_And_Init_Display(DISPLAY_LCD);

	//Open_Text_File(EBOOK_TEXT_FILE);

	//Set_Text_Detail(DEFAULT_HZK_FILE, DEFAULT_FREETYPE_FILE, DEFAULT_FONT_SIZE);
	
	//Show_Next_Page();

	All_Input_Device_Init();

	//Socket_Select_And_Init("udp", "192.168.0.3");

	//printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit:\n");
	//fflush(stdout);//刷新输出缓冲区，否则以上打印(末尾没有\n)不输出

	//Get_Format_Opr("bmp")->Get_Pic_Region("cancel.bmp", &tPicReg);

	Selected_Display()->Clean_Screen();

	//Lcd_Show_Pic(0, 0, &tPicReg);

	//Page_Mem_Prepare(5);

	//Get_Page_Opr("mainpage")->PrepareSelf();
	//Get_Page_Opr("mainpage")->Run();

	Page_Change("mainpage");

	while (1) {
	#if 0
		Input_Get_Key(&tInputEvent);
		if (tInputEvent.iVal == INPUT_VALUE_DOWN) {
			printf("show next page.\n");
			Socket_Send("show next page.\n");
			Show_Next_Page();
		} else if (tInputEvent.iVal == INPUT_VALUE_UP) {
			printf("show pre page.\n");
			Show_Pre_Page();
		} else if (tInputEvent.iVal == INPUT_VALUE_EXIT) {
			printf("quit.\n");
			return 0;
		}
	#endif

		//Get_Page_Opr("mainpage")->Get_Input_Event();
	}

	return 0;
}

