

struct disp_driver
{
	const char * disp_device_name;
	int (*disp_device_init)(struct platform_device *);
	int (*disp_device_clean)(struct platform_device *);
	int (*disp_device_remove)(struct platform_device *);
}
