#include "input_manager.h"
#include <stdio.h>
#include <errno.h>

#include "tslib.h"

#include "memwatch.h"

static T_Input_Opr g_tTouchscreenOpr;
struct tsdev *g_ptTsDev;

static int Touchscreen_Init(void)
{
	char *pcTsDevice = NULL;
	
	if ((pcTsDevice = getenv("TSLIB_TSDEVICE")) == NULL) {
		g_ptTsDev = ts_open(pcTsDevice, 0);
	} else {
		g_ptTsDev = ts_open("/dev/input/event0", 0);
	}

	if (!g_ptTsDev) {
		perror("ts_open");
		return -1;
	}

	if (ts_config(g_ptTsDev)) {
		perror("ts_config");
		ts_close(g_ptTsDev);
		return -1;
	}

	g_tTouchscreenOpr.iFd = ts_fd(g_ptTsDev);
	if (g_tTouchscreenOpr.iFd < 0) {
		perror("ts_fd");
		ts_close(g_ptTsDev);
		return -1;
	}
	return 0;
}

static void Touchscreen_Exit(void)
{
	ts_close(g_ptTsDev);
}

static int Touchscreen_Get_Data(PT_Input_Data ptInputData)
{
	struct ts_sample samp;
	int ret;
	
	ret = ts_read(g_ptTsDev, &samp, 1);
	if (ret < 0) {
		perror("ts_read");
		return -1;
	}

	ptInputData->iType = INPUT_TYPE_TOUCHSCREEN;
	ptInputData->iX = samp.x;
	ptInputData->iY = samp.y;
	ptInputData->dwPressure = samp.pressure;

	printf("%ld.%06ld: %6d %6d %6d\n", samp.tv.tv_sec, samp.tv.tv_usec, samp.x, samp.y, samp.pressure);
	return 1;
	//return 0;
}

static T_Input_Opr g_tTouchscreenOpr = {
	.c_pcName   = "touchscreen",
	.Input_Init = Touchscreen_Init,
	.Input_Exit = Touchscreen_Exit,
	.Input_Get_Data = Touchscreen_Get_Data,
};

int Touchscreen_Input_Init(void)
{
	return Input_Opr_Regisiter(&g_tTouchscreenOpr);
}

