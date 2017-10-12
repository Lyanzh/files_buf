#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>

int main(int argc, char **argv)
{
	Disp_Init();
	Font_Init();
	Encoding_Init();

	return 0;
}
