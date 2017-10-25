#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

static T_Input_Opr g_tButtonOpr;
int g_iFd;

static int Button_Init(void)
{
    g_iFd = open("/dev/buttons", O_RDWR);
    if (g_iFd < 0)
    {
        printf("error, can't open %s\n", "/dev/buttons");
        return -1;
    }

    g_tButtonOpr.iFd = g_iFd;
    
	return 0;
}

static void Button_Exit(void)
{
	close(g_iFd);
}

static int Button_Get_Data(PT_Input_Data ptInputData)
{
	char cGetKey;
	
	read(fd, &cGetKey, 1);
	if (cGetKey) {
		ptInputData->iType = INPUT_TYPE_BUTTON;
		ptInputData->cCode = cGetKey;
		return 1;
	}

	return 0;
}

static T_Input_Opr g_tButtonOpr = {
	.c_pcName   = "button",
	.Input_Init = Button_Init,
	.Input_Exit = Stdin_Exit,
	.Input_Get_Data = Button_Get_Data,
};

int Button_Input_Init(void)
{
	return Input_Opr_Regisiter(&g_tButtonOpr);
}

