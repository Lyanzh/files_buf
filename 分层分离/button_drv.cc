/* 分配/设置/注册一个platform_driver */

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>

struct pin_desc
{
	unsigned int irq;
	const char *name;
	unsigned int pin;
	unsigned char key_val;
};

struct pin_desc pins_desc[1];

static int major;

static struct class * cls;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
static volatile int button_pressed;

static char buttons_status = 0x0;

static irqreturn_t button_irq_handler(int this_irq, void *dev_id)
{
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;

	unsigned char val = 0;

	val = s3c2410_gpio_getpin(pindesc->pin);
	if(val)
		buttons_status = pindesc->key_val;
	else
		buttons_status = 0x80 | pindesc->key_val;

	printk("irq = %d\n", this_irq);
	printk("buttons_status = 0x%x\n", buttons_status);

	button_pressed = 1;
	wake_up_interruptible(&button_waitq);
	
	return IRQ_HANDLED;
}

static int s3c24xx_buttons_open(struct inode *inode, struct file *file)
{
	request_irq(pins_desc[0].irq, button_irq_handler, IRQT_BOTHEDGE, pins_desc[0].name, &pins_desc[0]);
	
    return 0;
}

static int s3c24xx_buttons_read(struct file *filp, char __user *buff, 
                                         size_t count, loff_t *offp)
{
	wait_event_interruptible(button_waitq, button_pressed);

	copy_to_user(buff, (const void *)&buttons_status, 1);

	button_pressed = 0;
   
	return 1;
}

static const struct file_operations button_fops = {
	.owner	= THIS_MODULE,
	.open	= s3c24xx_buttons_open,
	.read	= s3c24xx_buttons_read,
};

int button_probe(struct platform_device *pdev)
{
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_IO, 0);
	pins_desc[0].pin = res->start;
	pins_desc[0].key_val = res->end;
	pins_desc[0].name = res->name;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	pins_desc[0].irq = res->start;

	major = register_chrdev(0, "mybutton", &button_fops);

	cls = class_create(THIS_MODULE, "mybutton");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "mybutton");

	return 0;
}

int button_remove(struct platform_device *pdev)
{
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "mybutton");
	return 0;
}

struct platform_driver button_drv = 
{
	.probe = button_probe,
	.remove = button_remove,
	.driver = {
		.name = "mybutton",
	},
};

static int button_drv_init(void)
{
	platform_driver_register(&button_drv);
	return 0;
}

static void button_drv_exit(void)
{
	platform_driver_unregister(&button_drv);
}

module_init(button_drv_init);
module_exit(button_drv_exit);

MODULE_LICENSE("GPL");

