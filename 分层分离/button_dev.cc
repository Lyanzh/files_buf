#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <asm/arch/regs-gpio.h>

static struct resource button_resource[] = 
{
	[0] = {
		.start = S3C2410_GPF0,
		.end = 0x01,
		.name = "S2",
		.flags = IORESOURCE_IO,
	},
	[1] = {
		.start = IRQ_EINT0,
		.flags = IORESOURCE_IRQ,
	},
};

void button_release(struct device * dev)
{
	
}

static struct platform_device button_dev = {
	.name = "mybutton",
	.id = -1,
	.num_resources = ARRAY_SIZE(button_resource),
	.dev = {
		.release = button_release,
	},
};

static int button_dev_init(void)
{
	platform_device_register(&button_dev);
	return 0;
}

static void button_dev_exit(void)
{
	platform_device_unregister(&button_dev);
}

module_init(button_dev_init);
module_exit(button_dev_exit);
MODULE_LICENSE("GPL");
