#include "draw.h"
#include "encoding_manager.h"
#include "fonts_manager.h"
#include "disp_manager.h"
#include "input_manager.h"

#include "memwatch.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	char cOpr;

	Disp_Opr_Init();
	Font_Opr_Init();
	Encoding_Opr_Init();

	Input_Opr_Init();

	Show_Disp_Opr();
	Show_Font_Opr();
	Show_Encoding_Opr();
	Show_Input_Opr();

	Select_And_Init_Display("s3c2440-lcd");

	Open_Text_File("text_ansi.txt");

	printf("Open_Text_File.\n");

	Set_Text_Detail("HZK16", "simsun.ttc", 16);

	printf("Set_Text_Detail.\n");
	
	Show_Next_Page();

	printf("Show_Next_Page.\n");

	while (1) {
		printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: ");
		do {
			cOpr = getchar();
		} while (cOpr != 'n' && cOpr != 'u' &&cOpr != 'q');

		if (cOpr == 'n') {
			printf("show next page.\n");
			Show_Next_Page();
		} else if (cOpr == 'u') {
			printf("show pre page.\n");
			Show_Pre_Page();
		} else if (cOpr == 'q') {
			printf("quit.\n");
			return 0;
		}
	}

	return 0;
}

