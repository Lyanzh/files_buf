#include "config.h"
#include "draw.h"
#include "encoding_manager.h"
#include "fonts_manager.h"
#include "disp_manager.h"
#include "input_manager.h"

#include "memwatch.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	T_Input_Event tInputEvent;

	Disp_Opr_Init();
	Font_Opr_Init();
	Encoding_Opr_Init();
	Input_Opr_Init();

	Show_Disp_Opr();
	Show_Font_Opr();
	Show_Encoding_Opr();
	Show_Input_Opr();

	Select_And_Init_Display(DISPLAY_LCD);

	Open_Text_File("text_ansi.txt");

	Set_Text_Detail("HZK16", "simsun.ttc", 16);
	
	Show_Next_Page();

	printf("Show_Next_Page over.\n");

	All_Input_Device_Init();

	printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit:");
	fflush(stdout);//刷新输出缓冲区，否则以上打印(末尾没有\n)不输出

	while (1) {
	#if 1
		if (Input_Get_Key(&tInputEvent)) {
			if (tInputEvent.iVal == INPUT_VALUE_DOWN) {
				printf("show next page.\n");
				Show_Next_Page();
			} else if (tInputEvent.iVal == INPUT_VALUE_UP) {
				printf("show pre page.\n");
				Show_Pre_Page();
			} else if (tInputEvent.iVal == INPUT_VALUE_EXIT) {
				printf("quit.\n");
				return 0;
			}
		}
	#endif
	}

	return 0;
}

