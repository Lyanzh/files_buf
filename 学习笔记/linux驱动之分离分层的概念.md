# linux驱动之分离分层的概念 #

### bus_drv_dev模型： ###

![](http://i.imgur.com/NrUsfLa.png)

### 平台设备驱动： ###

- 包含BUS（总线)、DEVICE、DRIVER；

- DEVICE：硬件相关的代码；

- DRIVER：比较稳定的代码；

- BUS有一个driver链表和device链表；

### 简要的工作过程： ###

1. 把device放入bus的device链表中；

2. 从bus的drv链表中取出每一个drv，用bus的match函数判断drv能否支持dev；

3. 如果可以执行，则调用probe函数。

### device驱动程序代码： ###

	/*
	 * file name： led_dev.c
	 */
	#include <linux/platform_device.h>
	#include <asm/mach/arch.h>
	#include <asm/mach/map.h>
	
	
	/* 分配注册一个平台device结构体 */
	static struct resource led_resources[] = {
	    [0] = {
	        .start= 0x56000050,
	        .end= 0x56000050 + 8 - 1,
	        .flags= IORESOURCE_MEM,
	    },
	    [1] = {
	        .start= 5,  //第4个引脚
	        .end= 5,
	        .flags= IORESOURCE_IRQ,      
	    },
	};
	
	void led_release(struct device * dev)
	{
	}
	
	static struct platform_device led_dev = {
	    .name= "myled",
	    .id= -1,
	    .resource= led_resources,
	    .num_resources= ARRAY_SIZE(led_resources),
	    .dev = {.release = led_release,},
	};
	
	static int led_dev_init(void)
	{
	    platform_device_register(&led_dev);
	
	    return 0;
	}
	
	static void led_dev_exit(void)
	{
	    platform_device_unregister(&led_dev);
	    return ;
	}
	
	module_init(led_dev_init);
	module_exit(led_dev_exit);
	MODULE_LICENSE("GPL");

### driver驱动程序代码： ###

	/*
	 * file name:led_driver.c
	 */
	#include <linux/platform_device.h>
	#include <asm/mach/arch.h>
	#include <asm/mach/map.h>
	#include <linux/sched.h>
	#include <linux/signal.h>
	#include <linux/spinlock.h>
	#include <linux/errno.h>
	#include <linux/random.h>
	#include <linux/poll.h>
	#include <linux/init.h>
	#include <linux/slab.h>
	#include <linux/module.h>
	#include <linux/wait.h>
	#include <linux/mutex.h>
	#include <linux/io.h>
	#include <linux/fs.h>
	
	static int major;
	
	struct class *led_class;
	struct class_device *led_class_dev;
	static volatile unsigned long *gpio_con;
	static volatile unsigned long *gpio_dat;
	static int pin;
	
	static int led_open(struct inode *inode, struct file *file)
	{
	    *gpio_con &= ~(0x3<<(pin*2));
	    *gpio_con |= 0x1<<(pin*2);
	    return 0;
	}
	
	ssize_t led_write(struct file *file, const char  *buff, size_t len, loff_t *pos )
	{
	    int val;
	    copy_from_user(&val, buff, 1);
	    val &= 0x1;
	    *gpio_dat &= ~(1<<pin);
	    *gpio_dat |= (val<<pin);
	
	    return 0; 
	}
	
	struct file_operations led_fops = {
	    .owner = THIS_MODULE,
	    .open  = led_open,
	    .write = led_write,
	};
	
	
	static int led_probe(struct platform_device *pdev)
	{
	    struct resource *res;
	
	    printk("led drv probe\n");
	    
	    res = platform_get_resource(pdev,IORESOURCE_MEM,0);
	    gpio_con = ioremap(res->start,res->end-res->start + 1);
	    gpio_dat = gpio_con + 1;
	    
	    res = platform_get_resource(pdev,IORESOURCE_IRQ,0);
	    pin = res->start;
	    
	    major = register_chrdev(0,"myled", &led_fops );
	    led_class = class_create(THIS_MODULE,"myled");
	    led_class_dev = class_device_create(led_class,NULL,MKDEV(major,0),NULL,"led%d",pin);
	    
	    return 0;
	}
	
	
	static int led_remove(struct platform_device *pdev)
	{
	    iounmap(gpio_con);
	    class_device_unregister(led_class_dev);
	    class_destroy(led_class);
	    unregister_chrdev( major,"myled");
	    printk("led remove\n");
	    return 0;
	}
	
	struct platform_driver led_drv = {
	    .driver={
	        .owner=THIS_MODULE,
	        .name = "myled",
	    },
	    .probe = led_probe,
	    .remove= led_remove,
	};
	
	static int led_drv_init(void)
	{
	    platform_driver_register(&led_drv);
	    return 0;
	}
	
	static void led_drv_exit(void)
	{
	    platform_driver_unregister(&led_drv);
	}
	
	module_init(led_drv_init);
	module_exit(led_drv_exit);
	MODULE_LICENSE("GPL");

