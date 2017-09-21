## Linux字符设备驱动之同步互斥阻塞 ##

### 互斥的实现 ###

为了实现同一时绝对的只能有一个进程使用某个设备需要互斥机制，linux互斥机制有多种，如原子变量、互斥锁、信号量、自旋锁、读写锁等等。

1、原子操作

指执行的过程中不会被别的代码路径所中断的操作

1.1 其常使用的函数有：

    atomic_t v = ATOMIC_INIT(0);	//定义原子变量v并初始化为0
    atomic_read(atomic_t *v);		//返回原子变量的值
    void atomic_inc(atomic_t *v);	//原子变量增加1
    void atomic_dec(atomic_t *v);	//原子变量减少1
    int atomic_dec_and_test(atomic_t *v);	//自减操作后测试其是否为0，为0则返回true，否则返回false

1.2 使用过程

 （1）设置原子变量，用来标记设备是否被占用

如定义原子变量canopen：

    static atomic_t canopen = ATOMIC_INIT(1);     //定义原子变量并初始化为1

 （2）在打开设备之前先测试设备是否可用

假设我们用1表示可用，检测函数如下：

    if (！atomic_dec_and_test(&canopen))//检测，设备不可用执行if语句
    {
    	atomic_inc(&canopen);//将原子变量变回原来的值
    	return -EBUSY;
    }

设备可用时，原子变量减1后为0，检测函数返回ture，非ture为false，则不执行if语句，此时原子变量也变为0，表示设备不可用。

（3）在关闭设备的函数里最后把设备标记为未被占用。即使用

    atomic_inc(atomic_t *v)

让原子变量为1，表示设备可用。

使用原子变量，在原子变量加减的时候，不能其他操作打断，因而保证了绝对互斥，同一时间设备只能被一个进程使用。

2、信号量

信号量(Semaphore)，有时被称为信号灯，是在多线程环境下使用的一种设施, 它负责协调各个线程, 以保证它们能够正确、合理的使用公共资源。

只有获取到信号量的进程才能才能执行临界区代码，获取不到信号量的进程返回或休眠。

 2.1 使用函数

 定义信号量sem：

    struct semaphore sem;

初始化信号量：

    void sema_init (struct semaphore *sem, int val);
    void init_MUTEX(struct semaphore *sem); //初始化为0 

可用使用 DECLARE_MUTEX ，实现信号量的定义并初始化，如：

    DECLARE_MUTEX(name);  //定义一个信号量name，并初始化它的值为1

注意 init_MUTEX（），DECLARE_MUTEX 在 linux2.6.38 以后已被移除。其中 DECLARE_MUTEX(name) 被DEFINE_SEMAPHORE替代，为避免与DEFINE_MUTEX 互斥锁相近引起开发者误解。

获得信号量：

    void down(struct semaphore * sem);
    int down_interruptible(struct semaphore * sem);	//获取不到休眠，可被中断
    int down_trylock(struct semaphore * sem);		//获取不到立即返回

释放信号量：

    void up(struct semaphore * sem);

2.2 使用过程

（1）定义一个信号量。如，定义一个信号量button_lock：

    static DECLARE_MUTEX(button_lock);//定义互斥锁

（2）打开设备时先获得信号量：

	down(&button_lock);

也可用其他函数。第一个进程执行到down函数时可以申请到信号量button_lock，第二个进程如果再申请则申请不到，进入休眠状态，直到信号量被释放，第二个进程就可以进行。

（3）关闭设备最后释放信号量：

	up(&button_lock);

### 阻塞、非阻塞的实现 ###

1、 阻塞操作：是指在执行设备操作时若不能获得资源则挂起进程，直到满足可操作的条件后再进行操作。**被挂起的进程进入休眠状态**，被从调度器的运行队列移走，直到等待的条件被满足。

非阻塞操作：进程在不能进行设备操作时并不挂起，它或者放弃，或者不停地查询，直至可以进行操作为止。

2、 通过判断函数带入的参数 file 的 **file->f_flags** 来判断是否为阻塞，以做相应的操作。

    if (file->f_flags & O_NONBLOCK)
    {
    	//非阻塞，实现立即返回
    }
    else
    {
		//阻塞，实现休眠
	}

实现后，应用程序调用实现后的函数时，可以选择阻塞或非阻塞方式，阻塞时陷入休眠，非阻塞时立即返回信息。一般默认是阻塞，应用程序设置非阻塞方式打开使用 O_NONBLOCK ，如下：

    fd = open("...", O_RDWR | O_NONBLOCK);

### 示例程序 ###

#### 原子操作 ####

驱动程序

	#include <linux/module.h>
	#include <linux/kernel.h>
	#include <linux/fs.h>
	#include <linux/init.h>
	#include <linux/delay.h>
	#include <linux/irq.h>
	#include <asm/uaccess.h>
	#include <asm/irq.h>
	#include <asm/io.h>
	#include <asm/arch/regs-gpio.h>
	#include <asm/hardware.h>
	#include <linux/poll.h>

    static struct class *fifthdrv_class;
    static struct class_device *fifthdrv_class_dev;
    
    volatile unsigned long *gpfcon;
    volatile unsigned long *gpfdat;
    
    volatile unsigned long *gpgcon;
    volatile unsigned long *gpgdat;
    
    /*定义并初始化等待队列头*/
    static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
    
    /* 中断事件标志, 中断服务程序将它置1，fifth_drv_read将它清0 */
    static volatile int ev_press = 0;
    
    static struct fasync_struct *button_async;

    struct pin_desc{
    	unsigned int pin;
    	unsigned int key_val;
    };
    
    /* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
    /* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */
    static unsigned char key_val;

    struct pin_desc pins_desc[4] = {
    	{S3C2410_GPF0, 0x01},
    	{S3C2410_GPF2, 0x02},
    	{S3C2410_GPG3, 0x03},
    	{S3C2410_GPG11, 0x04},
    };

	static atomic_t canopen = ATOMIC_INIT(1);     //定义原子变量并初始化为1

	/*
	 * 确定按键值
	 */
	static irqreturn_t buttons_irq(int irq, void *dev_id)
	{
	    struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	    unsigned int pinval;
	    
	    pinval = s3c2410_gpio_getpin(pindesc->pin);
	
	    if (pinval)
	    {
	        /* 松开 */
	        key_val = 0x80 | pindesc->key_val;
	    }
	    else
	    {
	        /* 按下 */
	        key_val = pindesc->key_val;
	    }
	
	    ev_press = 1;                  /* 表示中断发生了 */
	    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	    
	    kill_fasync (&button_async, SIGIO, POLL_IN);
	    
	    return IRQ_RETVAL(IRQ_HANDLED);
	}

	static int fifth_drv_open(struct inode *inode, struct file *file)
	{

	    if (!atomic_dec_and_test(&canopen))	//减1后不等于0则返回false
	    {
	        atomic_inc(&canopen);	//自加1
	        return -EBUSY;
	    }

	    /* 配置GPF0,2为输入引脚 */
	    /* 配置GPG3,11为输入引脚 */
	    request_irq(IRQ_EINT0,  buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	    request_irq(IRQ_EINT2,  buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	    request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	    request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);
	
	    return 0;
	}

	ssize_t fifth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
	{
	    if (size != 1)
	        return -EINVAL;
	
	    /* 如果没有按键动作, 休眠 */
	    wait_event_interruptible(button_waitq, ev_press);
	
	    /* 如果有按键动作, 返回键值 */
	    copy_to_user(buf, &key_val, 1);
	    ev_press = 0;
	    
	    return 1;
	}

	int fifth_drv_close(struct inode *inode, struct file *file)
	{
	    /*释放原子变量，即加1，因为open的时候有减1测试操作*/
	    atomic_inc(&canopen);
	
	    free_irq(IRQ_EINT0, &pins_desc[0]);
	    free_irq(IRQ_EINT2, &pins_desc[1]);
	    free_irq(IRQ_EINT11, &pins_desc[2]);
	    free_irq(IRQ_EINT19, &pins_desc[3]);
	    return 0;
	}

	static unsigned fifth_drv_poll(struct file *file, poll_table *wait)
	{
	    unsigned int mask = 0;
	    poll_wait(file, &button_waitq, wait); // 不会立即休眠
	
	    if (ev_press)
	        mask |= POLLIN | POLLRDNORM;
	
	    return mask;
	}

	static int fifth_drv_fasync (int fd, struct file *filp, int on)
	{
	    printk("driver: fifth_drv_fasync\n");
	    return fasync_helper (fd, filp, on, &button_async);
	}

	static struct file_operations sencod_drv_fops = {
	    .owner   = THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	    .open    = fifth_drv_open,
	    .read    = fifth_drv_read,
	    .release = fifth_drv_close,
	    .poll    = fifth_drv_poll,
	    .fasync  = fifth_drv_fasync,
	};

	int major;
	static int fifth_drv_init(void)
	{
	    major = register_chrdev(0, "fifth_drv", &sencod_drv_fops);
	
	    fifthdrv_class = class_create(THIS_MODULE, "fifth_drv");
	
	    fifthdrv_class_dev = class_device_create(fifthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */
	
	    gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	    gpfdat = gpfcon + 1;
	
	    gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	    gpgdat = gpgcon + 1;
	
	    return 0;
	}

	static void fifth_drv_exit(void)
	{
	    unregister_chrdev(major, "fifth_drv");
	    class_device_unregister(fifthdrv_class_dev);
	    class_destroy(fifthdrv_class);
	    iounmap(gpfcon);
	    iounmap(gpgcon);
	    return 0;
	}

	module_init(fifth_drv_init);
	module_exit(fifth_drv_exit);

	MODULE_LICENSE("GPL");

测试程序

	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <stdio.h>
	#include <poll.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <fcntl.h>

	int fd;
	
	void my_signal_fun(int signum)
	{
	    unsigned char key_val;
	    read(fd, &key_val, 1);
	    printf("key_val: 0x%x\n", key_val);
	}
	
	int main(int argc, char **argv)
	{
	    unsigned char key_val;
	    int ret;
	    int Oflags;
	
	    signal(SIGIO, my_signal_fun);
	    
	    fd = open("/dev/buttons", O_RDWR);
	    if (fd < 0)
	    {
	        printf("can't open!\n");
	        return -1;
	    }
	
	    fcntl(fd, F_SETOWN, getpid());
	    
	    Oflags = fcntl(fd, F_GETFL); 
	    
	    fcntl(fd, F_SETFL, Oflags | FASYNC);
	
	
	    while (1)
	    {
	        sleep(1000);
	    }
	    
	    return 0;
	}

测试步骤：

	insmod key_interrupt_atomic.ko
	./test_atomic &
	./test_atomic
	
	can't open!

第二次运行测试程序打开失败。

#### 信号量和阻塞 ####

驱动程序

	#include <linux/module.h>
	#include <linux/kernel.h>
	#include <linux/fs.h>
	#include <linux/init.h>
	#include <linux/delay.h>
	#include <linux/irq.h>
	#include <asm/uaccess.h>
	#include <asm/irq.h>
	#include <asm/io.h>
	#include <asm/arch/regs-gpio.h>
	#include <asm/hardware.h>
	#include <linux/poll.h>
	
	static struct class *sixthdrv_class;
	static struct class_device *sixthdrv_class_dev;
	
	volatile unsigned long *gpfcon;
	volatile unsigned long *gpfdat;
	
	volatile unsigned long *gpgcon;
	volatile unsigned long *gpgdat;
	
	/*定义并初始化等待队列头*/
	static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
	
	/* 中断事件标志, 中断服务程序将它置1，sixth_drv_read将它清0 */
	static volatile int ev_press = 0;
	
	static struct fasync_struct *button_async;
	
	struct pin_desc{
	    unsigned int pin;
	    unsigned int key_val;
	};
	
	/* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
	/* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */
	static unsigned char key_val;
	
	struct pin_desc pins_desc[4] = {
	    {S3C2410_GPF0, 0x01},
	    {S3C2410_GPF2, 0x02},
	    {S3C2410_GPG3, 0x03},
	    {S3C2410_GPG11, 0x04},
	};
	
	/*定义一个信号量并初始化为1*/
	static DEFINE_SEMAPHORE(button_lock);
	
	/*
	 * 确定按键值
	 */
	static irqreturn_t buttons_irq(int irq, void *dev_id)
	{
	    struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	    unsigned int pinval;
	    
	    pinval = s3c2410_gpio_getpin(pindesc->pin);
	
	    if (pinval)
	    {
	        /* 松开 */
	        key_val = 0x80 | pindesc->key_val;
	    }
	    else
	    {
	        /* 按下 */
	        key_val = pindesc->key_val;
	    }
	
	    ev_press = 1;/* 表示中断发生了 */
	    wake_up_interruptible(&button_waitq);/* 唤醒休眠的进程 */
	    
	    kill_fasync (&button_async, SIGIO, POLL_IN);
	    
	    return IRQ_RETVAL(IRQ_HANDLED);
	}
	
	static int sixth_drv_open(struct inode *inode, struct file *file)
	{
	    /*非阻塞*/
	    if (file->f_flags & O_NONBLOCK)
	    {
	        if (down_trylock(&button_lock))	//获取信号量，失败返回非0
	            return -EBUSY;
	    }
	    else
	    {
	        /* 获取信号量 */
	        //如果无法获取信号量，则会休眠
	        down(&button_lock);
	    }
	
	    /* 配置GPF0,2为输入引脚 */
	    /* 配置GPG3,11为输入引脚 */
	    request_irq(IRQ_EINT0,  buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	    request_irq(IRQ_EINT2,  buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	    request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	    request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);
	
	    return 0;
	}
	
	ssize_t sixth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
	{
	    if (size != 1)
	        return -EINVAL;
	    /* 当没有按键按下时，休眠
	     * 即ev_press = 0;
	     * 当有按键按下时，发生中断，在中断处理函数会唤醒
	     * 即ev_press = 1;
	     * 唤醒后，接着继续将数据通过copy_to_user函数传递给应用程序
	     */
	    /* 非阻塞 */
	    if (file->f_flags & O_NONBLOCK)
	    {
	        if (!ev_press)	//没有按键按下直接返回
	            return -EAGAIN;
	    }
	    /* 阻塞 */
	    else
	    {
	        /* 如果没有按键动作, 休眠 */
	        wait_event_interruptible(button_waitq, ev_press);
	    }
	
	    /* 如果有按键动作, 返回键值 */
	    copy_to_user(buf, &key_val, 1);
	    /* 将ev_press清零 */
	    ev_press = 0;
	    
	    return 1;
	}
	
	
	int sixth_drv_close(struct inode *inode, struct file *file)
	{
	    //atomic_inc(&canopen);
	    free_irq(IRQ_EINT0, &pins_desc[0]);
	    free_irq(IRQ_EINT2, &pins_desc[1]);
	    free_irq(IRQ_EINT11, &pins_desc[2]);
	    free_irq(IRQ_EINT19, &pins_desc[3]);
	    /*释放互斥信号量*/
	    up(&button_lock);
	
	    return 0;
	}
	
	static unsigned sixth_drv_poll(struct file *file, poll_table *wait)
	{
	    unsigned int mask = 0;
	    poll_wait(file, &button_waitq, wait);// 不会立即休眠
	
	    if (ev_press)
	        mask |= POLLIN | POLLRDNORM;
	
	    return mask;
	}
	
	static int sixth_drv_fasync (int fd, struct file *filp, int on)
	{
	    printk("driver: sixth_drv_fasync\n");
	    return fasync_helper (fd, filp, on, &button_async);
	}
	
	static struct file_operations sencod_drv_fops = {
	    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	    .open    =  sixth_drv_open,
	    .read    =    sixth_drv_read,
	    .release =  sixth_drv_close,
	    .poll    =  sixth_drv_poll,
	    .fasync  =  sixth_drv_fasync,
	};
	
	int major;
	static int sixth_drv_init(void)
	{
	    major = register_chrdev(0, "sixth_drv", &sencod_drv_fops);
	
	    sixthdrv_class = class_create(THIS_MODULE, "sixth_drv");
	
	    sixthdrv_class_dev = class_device_create(sixthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */
	
	    gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	    gpfdat = gpfcon + 1;
	
	    gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	    gpgdat = gpgcon + 1;
	
	    return 0;
	}
	
	static void sixth_drv_exit(void)
	{
	    unregister_chrdev(major, "sixth_drv");
	    class_device_unregister(sixthdrv_class_dev);
	    class_destroy(sixthdrv_class);
	    iounmap(gpfcon);
	    iounmap(gpgcon);
	    return 0;
	}
	
	module_init(sixth_drv_init);
	module_exit(sixth_drv_exit);

	MODULE_LICENSE("GPL");

测试程序

	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <stdio.h>
	#include <poll.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <fcntl.h>
	
	/* sixthdrvtest 
	  */
	int fd;
	
	void my_signal_fun(int signum)
	{
	    unsigned char key_val;
	    read(fd, &key_val, 1);
	    printf("key_val: 0x%x\n", key_val);
	}
	
	int main(int argc, char **argv)
	{
	    unsigned char key_val;
	    int ret;
	    int Oflags;
	    
	    fd = open("/dev/buttons", O_RDWR | O_NONBLOCK);
		//fd = open("/dev/buttons", O_RDWR);//默认阻塞
	    if (fd < 0)
	    {
	        printf("can't open!\n");
	        return -1;
	    }
	
	    while (1)
	    {
	        ret = read(fd, &key_val, 1);
	        printf("key_val: 0x%x, ret = %d\n", key_val, ret);
	        sleep(5);
	    }
	    
	    return 0;
	}

信号量互斥（阻塞时）测试结果：

	./test_signal_block &
	 ./test_signal_block &
	root@freescale /opt$ busybox ps
	PID USER       VSZ STAT COMMAND
	   1 root      2188 S    init
	   2 root         0 SW   [kthreadd]
	2580 root      2368 S    -sh
	2590 root      1460 S    ./test_signal_block
	2591 root      1460 D    ./test_signal_block

由上面的测试结果可知：当多次执行./test_signal_block &时，进程2590的状态为S即睡眠状态（阻塞状态时未按下按键时休眠），2591的状态为D即僵死状态(无法获取信号量休眠),只有杀死2590，2591才能变成S.

非阻塞测试结果(与信号量无关)：

	key_val: 0x0, ret = -1
	key_val: 0x0, ret = -1
	key_val: 0x0, ret = -1
	key_val: 0x0, ret = -1
	key_val: 0x0, ret = -1

未按下按键5s打印获取一次，结果是直接返回的。