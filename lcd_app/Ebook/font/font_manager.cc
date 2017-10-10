#include "font_manager.h"
#include <stdio.h>

static struct font_device font_dev = {
	.font_name = "freetype",
	.font_init = freetype_init,
};

int disp_init(void)
{
	font_dev.font_init();
}