#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/irq.h>

//static int minor;

struct pin_desc
{
	unsigned int irq;
	char *name;
	unsigned int pin;
	unsigned char key_val;
};

struct pin_desc pins_desc[4] =
{
	{IRQ_EINT0,  "S2", S3C2410_GPF0,  0x01},
	{IRQ_EINT2,  "S3", S3C2410_GPF2,  0x02},
	{IRQ_EINT11, "S4", S3C2410_GPG3,  0x03},
	{IRQ_EINT19, "S5", S3C2410_GPG11, 0x04},
};

static struct timer_list buttons_timer;
static struct input_dev *buttons_dev;
struct pin_desc *irq_pd;

/* 定义一个信号量并初始化为1 */
static DEFINE_SEMAPHORE(button_lock);

static irqreturn_t button_irq_handler(int this_irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;

	/* 10ms后启动定时器 */
	mod_timer(&buttons_timer, jiffies+HZ/100);

	return IRQ_HANDLED;
}

void buttons_timer_function(unsigned long data)
{
	struct pin_desc *pindesc = irq_pd;
	unsigned int pinval;

	if(!pindesc)
		return;

	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if(pinval)
	{
		/* 松开 : 最后一个参数: 0-松开, 1-按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);
	}
	else
	{
		/* 按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}
}


/*
 * 执行insmod命令时就会调用这个函数 
 */
static int __init s3c24xx_buttons_init(void)
{
	int i = 0;

	/* 1. 分配一个input_dev结构体 */
	buttons_dev = input_allocate_device();//在内存中为输入设备结构体分配一个空间,并对其主要的成员进行了初始化

	/* 2. 设置 */
	//分别设置设备所产生的事件以及上报的按键值。Struct iput_dev中有两个成员，
	//一个是evbit.一个是keybit.分别用表示设备所支持的动作和键值。
	
	/* 2.1 能产生哪类事件 */
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);

	/* 2.2 能产生这类操作里的哪些事件: L,S,ENTER,LEFTSHIT */
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	/* 3. 注册 */
	input_register_device(buttons_dev);

	/* 4. 硬件相关的操作 */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);

	for(i = 0; i < 4; i++)
	{
		request_irq(pins_desc[i].irq, button_irq_handler, IRQT_BOTHEDGE, pins_desc[i].name, &pin_desc[i]);
	}
        
    printk(DEVICE_NAME " initialized\n");
    return 0;
}

/*
 * 执行rmmod命令时就会调用这个函数
 */
static void __exit s3c24xx_buttons_exit(void)
{
	int i =0;
	for(i = 0; i < 4; i++)
	{
		free_irq(pins_desc[i].irq, &pin_desc[i]);
	}

	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);
}

/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(s3c24xx_buttons_init);
module_exit(s3c24xx_buttons_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("http://www.100ask.net");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 LED Driver");
MODULE_LICENSE("GPL");

