#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/irq.h>

#define DEVICE_NAME     "buttons"  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */

static struct cdev buttons_cdev;
static struct class *buttons_class;
static struct class_device	*buttons_class_devs;

int major;

static char buttons_status = 0x0;

static unsigned long gpio_va;

#define GPIO_OFT(x) ((x) - 0x56000000)
#define GPFCON  (*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000050)))
#define GPFDAT  (*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000054)))
#define GPGCON  (*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000060)))
#define GPGDAT  (*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000064)))

struct pin_desc
{
	unsigned int pin;
	unsigned char key_val;
};

struct pin_desc pins_desc[4] =
{
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04},
};

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
static volatile int button_pressed;

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
#if 0
    // 配置3引脚为输出
    //s3c2410_gpio_cfgpin(S3C2410_GPF0, S3C2410_GPIO_INPUT);
    GPFCON &= ~(0x3<<(0*2));
    
    //s3c2410_gpio_cfgpin(S3C2410_GPF2, S3C2410_GPIO_INPUT);
    GPFCON &= ~(0x3<<(2*2));

    //s3c2410_gpio_cfgpin(S3C2410_GPG3, S3C2410_GPIO_INPUT);
    GPGCON &= ~(0x3<<(3*2));

    //s3c2410_gpio_cfgpin(S3C2410_GPG11, S3C2410_GPIO_INPUT);
    GPGCON &= ~(0x3<<(11*2));
#endif

	if (request_irq(IRQ_EINT0,  button_irq_handler, IRQT_BOTHEDGE, "S2", &pins_desc[0]))
		return -EAGAIN;
	if (request_irq(IRQ_EINT2,  button_irq_handler, IRQT_BOTHEDGE, "S3", &pins_desc[1]))
		return -EAGAIN;
	if (request_irq(IRQ_EINT11, button_irq_handler, IRQT_BOTHEDGE, "S4", &pins_desc[2]))
		return -EAGAIN;
	if (request_irq(IRQ_EINT19, button_irq_handler, IRQT_BOTHEDGE, "S5", &pins_desc[3]))
		return -EAGAIN;
	
    return 0;
}

static int s3c24xx_buttons_read(struct file *filp, char __user *buff, 
                                         size_t count, loff_t *offp)
{
	int ret = 1;
    wait_event_interruptible(button_waitq, button_pressed);

	if (copy_to_user(buff, (const void *)&buttons_status, 1))
		ret = -EFAULT;

	button_pressed = 0;
   
    return ret;
}

static ssize_t s3c24xx_buttons_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    return 1;
}

static int s3c24xx_buttons_release(struct inode * inode, struct file * file)
{
	free_irq(IRQ_EINT0,  &pins_desc[0]);
	free_irq(IRQ_EINT2,  &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);

    return 0;
}

static struct file_operations s3c24xx_buttons_fops = {
    .owner   = THIS_MODULE,
    .open    = s3c24xx_buttons_open,
	.read    = s3c24xx_buttons_read,
	.write   = s3c24xx_buttons_write,
	.release = s3c24xx_buttons_release,
};

static int __init s3c24xx_buttons_init(void)
{
	int ret;
	dev_t devid;

    gpio_va = (unsigned long)ioremap(0x56000000, 0x100000);
	if (!gpio_va) {
		return -EIO;
	}

#if 0
    major = register_chrdev(0, DEVICE_NAME, &s3c24xx_buttons_fops);
    if (major < 0) {
      printk(DEVICE_NAME " can't register major number\n");
      return major;
    }
#endif

	if (major) {
		devid = MKDEV(major, 0);
		ret = register_chrdev_region(devid, 1, "buttons");
	} else {
		ret = alloc_chrdev_region(&devid, 0, 1, "buttons");
		major = MAJOR(devid);
	}
	if (ret < 0) {
		printk(KERN_ERR "buttons: couldn't register device number\n");
		goto out;
	}

	cdev_init(&buttons_cdev, &s3c24xx_buttons_fops);
	cdev_add(&buttons_cdev, devid, 1);

	buttons_class = class_create(THIS_MODULE, "buttons");
	if (IS_ERR(buttons_class)) {
		ret = PTR_ERR(buttons_class);
		printk(KERN_ERR "buttons: couldn't create class\n");
		goto out_chrdev;
	}

	buttons_class_devs = class_device_create(buttons_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if (unlikely(IS_ERR(buttons_class_devs))) {
		ret = PTR_ERR(buttons_class_devs);
		printk(KERN_ERR "buttons: couldn't create class devices\n");
		goto out_class;
	}
        
    printk(DEVICE_NAME " initialized\n");
    return 0;

out_class:
	class_destroy(buttons_class);

out_chrdev:
	unregister_chrdev_region(MKDEV(major, 0), 1);

out:
	return ret;
}

static void __exit s3c24xx_buttons_exit(void)
{
	class_device_destroy(buttons_class, MKDEV(major, 0));
	class_destroy(buttons_class);

	cdev_del(&buttons_cdev);
	unregister_chrdev_region(MKDEV(major, 0), 1);

    iounmap((void *)gpio_va);
}

module_init(s3c24xx_buttons_init);
module_exit(s3c24xx_buttons_exit);

MODULE_AUTHOR("Lin Yanzhou");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 BUTTON Driver");
MODULE_LICENSE("GPL");

