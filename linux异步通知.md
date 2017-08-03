## linux异步通知 ##

poll机制可实现有数据的时候就去读，没有数据的时候，如果超过规定一个时间，就表示超时时间。poll机制需要应用程序主动去读，而异步通知机制并不需要，当驱动程序有数据时，主动去通知应用程序来读取数据。

问：如何实现异步通知，有哪些要素？

答：有四个要素：

一、应用程序要实现有：注册信号处理函数，使用signal函数

二、谁来发？驱动来发

三、发给谁？发给应用程序，但应用程序必须告诉驱动PID

四、怎么发？驱动程序使用kill_fasync函数

问：应该在驱动的哪里调用kill_fasync函数？

答：kill_fasync函数的作用是，当有数据时去通知应用程序，理所当然的应该在用户终端处理函数里调用。

问：file_operations需要添加什么函数指针成员吗？

答：要的，需要添加fasync函数指针，要实现这个函数指针，幸运的是，这个函数仅仅调用了fasync_helper函数，而且这个函数是内核帮我们实现好了，驱动工程师不用修改，fasync_helper函数的作用是初始化/释放fasync_struct



详细请参考驱动源码：

    #include <linux/kernel.h>  
    #include <linux/fs.h>  
    #include <linux/init.h>  
    #include <linux/delay.h>  
    #include <linux/irq.h>  
    #include <asm/uaccess.h>  
    #include <asm/irq.h>  
    #include <asm/io.h>  
    #include <linux/module.h>  
    #include <linux/device.h> //class_create  
    #include <mach/regs-gpio.h>   //S3C2410_GPF1  
    //#include <asm/arch/regs-gpio.h>
    #include <mach/hardware.h>  
    //#include <asm/hardware.h>  
    #include <linux/interrupt.h>  //wait_event_interruptible  
    #include <linux/poll.h>   //poll  
    #include <linux/fcntl.h>  
  
  
    /* 定义并初始化等待队列头 */  
    static DECLARE_WAIT_QUEUE_HEAD(button_waitq);  
  
  
    static struct class *fifthdrv_class;  
    static struct device *fifthdrv_device;  
  
    static struct pin_desc{  
    unsigned int pin;  
    unsigned int key_val;  
    };  
  
    static struct pin_desc pins_desc[4] = {  
    {S3C2410_GPF1,0x01},  
    {S3C2410_GPF4,0x02},  
    {S3C2410_GPF2,0x03},  
    {S3C2410_GPF0,0x04},  
    };   
  
    static int ev_press = 0;  
  
    /* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */  
    /* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */  
    static unsigned char key_val;  
    int major;  
  
    static struct fasync_struct *button_fasync;  
  
    /* 用户中断处理函数 */  
    static irqreturn_t buttons_irq(int irq, void *dev_id)  
    {  
    struct pin_desc *pindesc = (struct pin_desc *)dev_id;  
    unsigned int pinval;  
    pinval = s3c2410_gpio_getpin(pindesc->pin);  
  
    if(pinval)  
    {  
    /* 松开 */  
    key_val = 0x80 | (pindesc->key_val);  
    }  
    else  
    {  
    /* 按下 */  
    key_val = pindesc->key_val;  
    }  
  
    ev_press = 1;/* 表示中断已经发生 */  
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */  
  
    /* 用kill_fasync函数告诉应用程序，有数据可读了  
     * button_fasync结构体里包含了发给谁(PID指定) 
     * SIGIO表示要发送的信号类型 
     * POLL_IN表示发送的原因(有数据可读了) 
     */  
    kill_fasync(&button_fasync, SIGIO, POLL_IN);  
    return IRQ_HANDLED;  
    }  
    static int fifth_drv_open(struct inode * inode,     struct file * filp)  
    {  
    /*  K1 ---- EINT1,K2 ---- EINT4,K3 ---- EINT2,K4 ---- EINT0 
     *  配置GPF1、GPF4、GPF2、GPF0为相应的外部中断引脚 
     *  IRQT_BOTHEDGE应该改为IRQ_TYPE_EDGE_BOTH 
     */  
    request_irq(IRQ_EINT1, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K1",&pins_desc[0]);  
    request_irq(IRQ_EINT4, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K2",&pins_desc[1]);  
    request_irq(IRQ_EINT2, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K3",&pins_desc[2]);  
    request_irq(IRQ_EINT0, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K4",&pins_desc[3]);  
    return 0;  
    }  
  
    static ssize_t fifth_drv_read(struct file *file, char __user *user, size_t size,loff_t *ppos)  
    {  
    if (size != 1)  
    return -EINVAL;  
  
    /* 当没有按键按下时，休眠。 
     * 即ev_press = 0; 
     * 当有按键按下时，发生中断，在中断处理函数会唤醒 
     * 即ev_press = 1;  
     * 唤醒后，接着继续将数据通过copy_to_user函数传递给应用程序 
     */  
    wait_event_interruptible(button_waitq, ev_press);  
    copy_to_user(user, &key_val, 1);  
  
    /* 将ev_press清零 */  
    ev_press = 0;  
    return 1; 
    }  
  
    static int fifth_drv_close(struct inode *inode, struct file *file)  
    {  
    free_irq(IRQ_EINT1,&pins_desc[0]);  
    free_irq(IRQ_EINT4,&pins_desc[1]);  
    free_irq(IRQ_EINT2,&pins_desc[2]);  
    free_irq(IRQ_EINT0,&pins_desc[3]);  
    return 0;  
    }
  
    static unsigned int fifth_drv_poll(struct file *file, poll_table *wait)  
    {  
    unsigned int mask = 0;  
      
    /* 该函数，只是将进程挂在button_waitq队列上，而不是立即休眠 */  
    poll_wait(file, &button_waitq, wait);  
      
    /* 当没有按键按下时，即不会进入按键中断处理函数，此时ev_press = 0  
     * 当按键按下时，就会进入按键中断处理函数，此时ev_press被设置为1 
     */  
    if(ev_press)  
    {  
    mask |= POLLIN | POLLRDNORM;  /* 表示有数据可读 */  
    }  
      
    /* 如果有按键按下时，mask |= POLLIN | POLLRDNORM,否则mask = 0 */  
    return mask;
    }  
  
    /* 当应用程序调用了fcntl(fd, F_SETFL, Oflags | FASYNC);  
     * 则最终会调用驱动的fasync函数，在这里则是fifth_drv_fasync 
     * fifth_drv_fasync最终又会调用到驱动的fasync_helper函数 
     * fasync_helper函数的作用是初始化/释放fasync_struct 
     */  
    static int fifth_drv_fasync(int fd, struct file *filp, int on)  
    {  
    return fasync_helper(fd, filp, on, &button_fasync);  
    }  
      
    /* File operations struct for character device */  
    static const struct file_operations fifth_drv_fops = {  
    .owner  = THIS_MODULE,  
    .open   = fifth_drv_open,  
    .read   = fifth_drv_read,  
    .release= fifth_drv_close,  
    .poll   = fifth_drv_poll,  
    .fasync = fifth_drv_fasync,  
    };  
      
      
    /* 驱动入口函数 */  
    static int fifth_drv_init(void)  
    {  
    /* 主设备号设置为0表示由系统自动分配主设备号 */  
    major = register_chrdev(0, "fifth_drv", &fifth_drv_fops);  
      
    /* 创建fifthdrv类 */  
    fifthdrv_class = class_create(THIS_MODULE, "fifthdrv");  
      
    /* 在fifthdrv类下创建buttons设备，供应用程序打开设备*/  
    fifthdrv_device = device_create(fifthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons");  
      
    return 0;  
    }  
      
    /* 驱动出口函数 */  
    static void fifth_drv_exit(void)  
    {  
    unregister_chrdev(major, "fifth_drv");  
    device_unregister(fifthdrv_device);  //卸载类下的设备  
    class_destroy(fifthdrv_class);  //卸载类  
    }  
      
    module_init(fifth_drv_init);  //用于修饰入口函数  
    module_exit(fifth_drv_exit);  //用于修饰出口函数  
      
    MODULE_AUTHOR("LWJ");  
    MODULE_DESCRIPTION("Just for Demon");  
    MODULE_LICENSE("GPL");  //遵循GPL协议

应用测试程序源码：

    #include <stdio.h>  
    #include <sys/types.h>  
    #include <sys/stat.h>  
    #include <fcntl.h>  
    #include <unistd.h>   //sleep  
    #include <poll.h>  
    #include <signal.h>  
    #include <fcntl.h>  
  
    int fd;  
      
    void mysignal_fun(int signum)  
    {  
    unsigned char key_val;  
    read(fd,&key_val,1);  
    printf("key_val = 0x%x\n",key_val);  
    }  
      
      
    /* fifth_test 
     */   
    int main(int argc ,char *argv[])  
    {  
    int flag;  
    signal(SIGIO,mysignal_fun);  
      
    fd = open("/dev/buttons",O_RDWR);  
    if (fd < 0)  
    {  
    printf("open error\n");  
    }  
      
    /* F_SETOWN:  Set the process ID 
     *  告诉内核，发给谁 
     */  
    fcntl(fd, F_SETOWN, getpid());  
      
    /*  F_GETFL :Read the file status flags 
     *  读出当前文件的状态 
     */  
    flag = fcntl(fd,F_GETFL);  
      
    /* F_SETFL: Set the file status flags to the value specified by arg 
     * int fcntl(int fd, int cmd, long arg); 
     * 修改当前文件的状态，添加异步通知功能 
     */  
    fcntl(fd,F_SETFL,flag | FASYNC);  
      
    while(1)  
    {  
    /* 为了测试，主函数里，什么也不做 */  
    sleep(1000);  
    }  
    return 0;  
    }

当无按键按下时，应用测试程序一直在sleep，当有按键按下时，signal会被调用，最终会调用mysignal_fun，在此函数里read(fd,&key_val,1);会去读出按键值，这样一来，应用程序就相当于不用主动去读数据了，每当驱动里有数据时，就会告诉应用程序有数据了，你该去读数据了，此时read函数才会被调用。


最后总结一下：

为了使设备支持异步通知机制，驱动程序中涉及以下3项工作：
1. 支持F_SETOWN命令，能在这个控制命令处理中设置filp->f_owner为对应进程ID。
   不过此项工作已由内核完成，设备驱动无须处理。
2. 支持F_SETFL命令的处理，每当FASYNC标志改变时，驱动程序中的fasync()函数将得以执行。
   驱动中应该实现fasync()函数。
3. 在设备资源可获得时，调用kill_fasync()函数激发相应的信号

应用程序：
fcntl(fd, F_SETOWN, getpid());  // 告诉内核，发给谁

Oflags = fcntl(fd, F_GETFL);   
fcntl(fd, F_SETFL, Oflags | FASYNC);  // 改变fasync标记，最终会调用到驱动的faync > fasync_helper：初始化/释放fasync_struct