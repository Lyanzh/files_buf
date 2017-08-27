#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/irq.h>

#define DEVICE_NAME     "buttons"  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */


static struct class *buttons_class;
static struct class_device *buttons_class_devs;

int major;

static char buttons_status = 0x0;

//static int minor;
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
	{S3C2410_GPF0,  0x01},
	{S3C2410_GPF2,  0x02},
	{S3C2410_GPG3,  0x03},
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

/* 应用程序对设备文件/dev/buttons执行open(...)时，
 * 就会调用s3c24xx_buttons_open函数
 */
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

	request_irq(IRQ_EINT0,  button_irq_handler, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2,  button_irq_handler, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, button_irq_handler, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, button_irq_handler, IRQT_BOTHEDGE, "S5", &pins_desc[3]);
	
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

static unsigned int s3c24xx_buttons_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

    /*把调用poll 或者select 的进程挂入队列，以便被驱动程序唤醒*/
	poll_wait(file, &button_waitq, wait);

	if (button_pressed)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations s3c24xx_buttons_fops = {
    .owner   = THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    = s3c24xx_buttons_open,
	.read    = s3c24xx_buttons_read,
	.write   = s3c24xx_buttons_write,
	.poll    = s3c24xx_buttons_poll,
	.release = s3c24xx_buttons_release,
};

/*
 * 执行insmod命令时就会调用这个函数 
 */
static int __init s3c24xx_buttons_init(void)
{
	int minor = 0;

    gpio_va = ioremap(0x56000000, 0x100000);
	if (!gpio_va) {
		return -EIO;
	}

    /* 注册字符设备
     * 参数为主设备号、设备名字、file_operations结构；
     * 这样，主设备号就和具体的file_operations结构联系起来了，
     * 操作主设备为LED_MAJOR的设备文件时，就会调用s3c24xx_leds_fops中的相关成员函数
     * LED_MAJOR可以设为0，表示由内核自动分配主设备号
     */
    major = register_chrdev(0, DEVICE_NAME, &s3c24xx_buttons_fops);
    if (major < 0) {
      printk(DEVICE_NAME " can't register major number\n");
      return major;
    }

	buttons_class = class_create(THIS_MODULE, "buttons");
	if (IS_ERR(buttons_class))
		return PTR_ERR(buttons_class);

	buttons_class_devs = class_device_create(buttons_class, NULL, MKDEV(major, 0), NULL, "buttons");
	
	if (unlikely(IS_ERR(buttons_class_devs)))
		return PTR_ERR(buttons_class_devs);
        
    printk(DEVICE_NAME " initialized\n");
    return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数
 */
static void __exit s3c24xx_buttons_exit(void)
{
    /* 卸载驱动程序 */
    unregister_chrdev(major, DEVICE_NAME);

	class_device_unregister(buttons_class_devs);
	class_destroy(buttons_class);
    iounmap(gpio_va);
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(s3c24xx_buttons_init);
module_exit(s3c24xx_buttons_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("http://www.100ask.net");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 LED Driver");
MODULE_LICENSE("GPL");



