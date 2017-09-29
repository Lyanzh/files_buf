

static struct disp_driver fb_driver = {
	.disp_device_init   = fb_init,
	.disp_device_clean  = fb_clean,
	.disp_device_remove = fb_remove,
	.disp_device_name	 = "s3c2440-lcd",
};

int disp_init()
{
	
}

