# Linux字符设备驱动之定时器去抖动按键驱动 #

## 按键防抖动的原理 ##

当按一次按健时，由于按健有反应时间、有抖动，可能按一次机器感应到几次，防抖就是让在按键正常反应时间内机器只感应一次按键效果，防止误操作。

当按键发生时产生中断，在中断服务程序中修改一个定时器的定时时间为10ms,并从10ms开始重新计数。定时器时间到后产生定时器中断，按键处理放在定时器中断里面。这样当10ms内有多个中断发生时只有最后一个中断起作用，以达到消除抖动的目的。

## linux内核定时器编程要点 ##

### 内核定时器的要素 ###

- 超时时间

- 处理函数

### 内核定时器的结构 ###

	struct timer_list {
	    struct list_head entry;
	    unsigned long expires;
	    void (*function)(unsigned long);
	    unsigned long data;
	    struct tvec_base *base;
	    .....
	};

### void (*function)(unsigned long data)的参数 ###

由 timer_list.data 传递，如果需要向 function 传递参数时，则应该设置 timer_list.data，否则可以不设置

### 与内核定时器相关的操作函数 ###

1. 使用init_timer函数初始化定时器

2. 设置timer_list.function，并实现这个函数指针

3. 使用add_timer函数向内核注册一个定时器

4. 使用mod_timer修改定时器时间，并启动定时器

### 设置超时时间 ###

    int mod_timer(struct timer_list *timer, unsigned long expires)

第二个参数为超时时间，怎么设置超时时间，如果定时为10ms，则一般的形式为: jiffies + (HZ /100)，HZ 表示100个jiffies，jiffies的单位为10ms，即HZ = 100*10ms = 1s

### 驱动代码相关部分 ###

	struct pin_desc *irq_pindes;
	
	static struct timer_list buttons_timer;  /* 定义一个定时器结构体 */
	
	/* 用户中断处理函数 */
	static irqreturn_t buttons_irq(int irq, void *dev_id)
	{
	    int ret;
	    irq_pindes = (struct pin_desc *)dev_id;
	    
	    /* 修改定时器定时时间，定时10ms，即10秒后启动定时器
	     * HZ 表示100个jiffies，jiffies的单位为10ms，即HZ = 100*10ms = 1s
	     * 这里HZ/100即定时10ms
	     */
	    ret = mod_timer(&buttons_timer, jiffies + (HZ /100));
	    if(ret == 1)
	    {
	        printk("mod timer success\n");
	    }
	    return IRQ_HANDLED;
	}
	
	/* 定时器处理函数 */
	static void buttons_timer_function(unsigned long data)
	{
	    struct pin_desc *pindesc = irq_pindes;
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
	
	    ev_press = 1;                           /* 表示中断已经发生 */
	    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	
	    /* 用kill_fasync函数告诉应用程序，有数据可读了
	     * button_fasync结构体里包含了发给谁(PID指定)
	     * SIGIO表示要发送的信号类型 
	     * POLL_IN表示发送的原因(有数据可读了) 
	     */  
	    kill_fasync(&button_fasync, SIGIO, POLL_IN);
	}  
	
	/* 驱动入口函数 */
	static int sixth_drv_init(void)
	{
	    /* 初始化定时器 */
	    init_timer(&buttons_timer);
	    /* 当定时时间到达时uttons_timer_function就会被调用 */
	    buttons_timer.function  = buttons_timer_function;
	    /* 向内核注册一个定时器 */
	    add_timer(&buttons_timer);
	      
	    /* 主设备号设置为0表示由系统自动分配主设备号 */  
	    major = register_chrdev(0, "sixth_drv", &sixth_drv_fops);
	
	    /* 创建sixthdrv类 */  
	    sixthdrv_class = class_create(THIS_MODULE, "sixthdrv");
	
	    /* 在sixthdrv类下创建buttons设备，供应用程序打开设备*/
	    sixthdrv_device = device_create(sixthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons");
	
	    return 0;
	}

