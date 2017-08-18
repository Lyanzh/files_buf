# Linux字符设备驱动程序开发 #

## 字符设备驱动程序中重要的数据结构和函数 ##

Linux操作系统将所有的设备（而不仅是存储器里的文件）都看成文件，以操作文件的方式访问设备。应用程序不能直接操作硬件，而是使用统一的接口函数调用硬件驱动程序。这组接口被称为系统调用，在库函数中定义。可以在glibc的fcntl.h、unistd.h、sys/ioctl.h等文件中看到如下定义，这些文件也可以在交叉编译工具链的/usr/local/arm/3.4.1/include目录下找到。

![](http://i.imgur.com/ffumxHq.jpg)

对于上述每个系统调用，驱动程序中都有一个与之对应的函数。对于字符设备驱动程序，这些函数集合在一个file_operations类型的数据结构中。file_operations结构在Linux内核的include/linux/fs.h文件中定义。

![](http://i.imgur.com/pPgOptt.jpg)

- 当应用程序使用open函数打开某个设备时，设备驱动程序的file_operations结构中的open成员就会被调用；

- 当应用程序使用read、write、ioctl等函数读写、控制设备时，驱动程序的file_operations结构中的相应成员（read、write、ioctl等）就会被调用。

从这个角度来说，编写字符设备驱动程序就是为具体硬件的file_operations结构编写各个函数（并不需要全部实现file_operations结构中的成员）。

那么，当应用程序通过open、read、write等系统调用访问某个设备文件时，Linux系统怎么知道去调用哪个驱动程序的file_operations结构中的open、read、write等成员呢？

### 设备文件有主/次设备号 ###

设备文件分为字符设备、块设备，比如PC机上的串口属于字符设备，硬盘属于块设备。在PC上运行命令“ls /dev/ttyS0 /dev/hda1-l”可以看到：

![](http://i.imgur.com/dDBEBDv.jpg)

### 模块初始化时，将主设备号与file_operations结构一起向内核注册 ###

驱动程序有一个初始化函数，在安装驱动程序时会调用它。在初始化函数中，会将驱动程序的file_operations结构连同其主设备号一起向内核进行注册。对于字符设备使用如下以下函数进行注册：

    int register_chrdev(unsigned int major, const char * name, struct file_operations *fops);

这样，应用程序操作设备文件时，Linux系统就会根据**设备文件的类型**（是字符设备还是块设备）、**主设备号找到在内核中注册的file_operations结构**（对于块设备为block_device_ operations结构），**次设备号供驱动程序自身用来分辨它是同类设备中的第几个**。

编写字符驱动程序的过程大概如下：



1. 编写驱动程序初始化函数

	进行必要的初始化，包括硬件初始化（也可以放其他地方）、向内核注册驱动程序等；



2. 构造file_operations结构中要用到的各个成员函数

实际的驱动程序当然比上述两个步骤复杂，但这两个步骤已经可以让我们编写比较简单的驱动程序，比如LED控制。其他比较高级的技术，比如中断、select机制、fasync异步通知机制，将在其他章节的例子中介绍。

## LED驱动程序源码分析 ##

本节以一个简单的LED驱动程序作为例子，让读者初步了解驱动程序的开发。开发板使用引脚GPB5～8外接4个LED：

![](http://i.imgur.com/JPqp6sI.jpg)

### LED驱动程序代码分析 ###

下面按照函数调用的顺序进行讲解

模块的初始化函数和卸载函数如下：

    /*
     * 执行insmod命令时就会调用这个函数 
     */
    static int __init s3c24xx_leds_init(void)
    //static int __init init_module(void)
    {
    	int ret;
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
    	ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &s3c24xx_leds_fops);
    	if (ret < 0) {
    		printk(DEVICE_NAME " can't register major number\n");
    		return ret;
    	}

    	leds_class = class_create(THIS_MODULE, "leds");
    	if (IS_ERR(leds_class))
    		return PTR_ERR(leds_class);

    	leds_class_devs[0] = class_device_create(leds_class, NULL, MKDEV(LED_MAJOR, 0), NULL, "leds");
    
    	for (minor = 1; minor < 4; minor++)
    	{
    		leds_class_devs[minor] = class_device_create(leds_class, NULL, MKDEV(LED_MAJOR, minor), NULL, "led%d", minor);
    		if (unlikely(IS_ERR(leds_class_devs[minor])))
    			return PTR_ERR(leds_class_devs[minor]);
    	}
    
    	printk(DEVICE_NAME " initialized\n");
    	return 0;
    }

    /*
     * 执行rmmod命令时就会调用这个函数 
     */
    static void __exit s3c24xx_leds_exit(void)
    //static void __exit cleanup_module(void)
    {
    	int minor;
    	/* 卸载驱动程序 */
    	unregister_chrdev(LED_MAJOR, DEVICE_NAME);

    	for (minor = 0; minor < 4; minor++)
    	{
    		class_device_unregister(leds_class_devs[minor]);
    	}
    	class_destroy(leds_class);
    	iounmap(gpio_va);
    }
    
    /* 这两行指定驱动程序的初始化函数和卸载函数 */
    module_init(s3c24xx_leds_init);
    module_exit(s3c24xx_leds_exit);

最后两行用来指明装载、卸载模块时所调用的函数。也可以不使用这两行，但是需要将这两个函数的名字改为init_module、cleanup_module。

执行“insmod s3c24xx_leds.ko”命令时就会调用s3c24xx_leds_init函数，这个函数核心的代码只有这一行:

    ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &s3c24xx_leds_fops);

它调用register_chrdev函数向内核注册驱动程序：将主设备号LED_MAJOR与file_operations结构s3c24xx_leds_fops联系起来。以后应用程序操作主设备号为LED_MAJOR的设备文件时，比如open、read、write、ioctl，s3c24xx_leds_fops中的相应成员函数就会被调用。

但是，s3c24xx_leds_fops中并不需要全部实现这些函数，用到哪个就实现哪个。

执行“rmmod s3c24xx_leds.ko”命令时就会调用s3c24xx_leds_exit函数，它进而调用unregister_chrdev函数卸载驱动程序，它的功能与register_chrdev函数相反。

s3c24xx_leds_init、s3c24xx_leds_exit函数前的“_ _init”、“_ _exit”只有在将驱动程序静态链接进内核时才有意义。前者表示s3c24xx_leds_init函数的代码被放在“.init.text”段中，这个段在使用一次后被释放（这可以节省内存）；后者表示s3c24xx_leds_exit函数的代码被放在“.exit.data”段中，在连接内核时这个段没有使用，因为不可能卸载静态键接的驱动程序。

下面来看看s3c24xx_leds_fops的组成：

    /* 这个结构是字符设备驱动程序的核心
     * 当应用程序操作设备文件时所调用的open、read、write等函数，
     * 最终会调用这个结构中指定的对应函数
     */
    static struct file_operations s3c24xx_leds_fops = {
    	.owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    	.open   =   s3c24xx_leds_open,
    	.read	=	s3c24xx_leds_read,
    	.write	=	s3c24xx_leds_write,
    	.ioctl	=	s3c24xx_leds_ioctl,
    };

宏THIS_MODULE在include／linux／module.h中定义如下，_ _this_module变量在编译模块时自动创建，无需关注这点。

    #define THIS_MODULE (&__this_module)

ile_operations类型的s3c24xx_leds_fops结构是驱动中最重要的数据结构，编写字符设备驱动程序的主要工作也是填充其中的各个成员。比如本驱动程序中用到open、ioctl成员被设为s3c24xx_leds_open、s3c24xx_leds_ioctl函数，前者用来初始化LED所用的GPIO引脚，后者用来根据用户传入的参数设置GPIO的输出电平。

s3c24xx_leds_open函数的代码如下：

    /* 应用程序对设备文件/dev/leds执行open()时，
     * 就会调用 s3c24xx_leds_open 函数
     */
    static int s3c24xx_leds_open(struct inode *inode, struct file *file)
    {
    	int i;
    	for (i = 0; i < 4; i++) {
    		//设置GPIO引脚的功能：本驱动中LED所涉及的GPIO引脚设为输出功能
    		s3c2410_gpio_cfgpin(led_table[i], led_cfg_table[i]);
    	}
    	return 0;
    }

在应用程序执行open(“/dev/leds”,...)系统调用时，s3c24xx_leds_open函数将被调用。它用来将LED所涉及的GPIO引脚设为输出功能。不在模块的初始化函数中进行这些设置的原因是：虽然加载了模块，但是这个模块却不一定会被用到，就是说这些引脚不一定用于这些用途，它们可能在其他模块中另作他用。所以，在使用时才去设置它，我们把对引脚的初始化放在open操作中。

s3c2410_gpio_cfgpin函数是内核里实现的，它被用来选择引脚的功能。

s3c24xx_leds_ioctl函数的代码如下：

    /* 应用程序对设备文件/dev/leds执行 ioctl() 时，
     * 就会调用 s3c24xx_leds_ioctl 函数
     */
    static int s3c24xx_leds_ioctl(
    	struct inode *inode,
    	struct file *file,
    	unsigned int cmd,
    	unsigned long arg)
    {
    	if (arg > 4) {
    		return -EINVAL;
    	}
    
    	switch (cmd) {
    	case IOCTL_LED_ON:
    		// 设置指定引脚的输出电平为0
    		s3c2410_gpio_setpin(led_table[arg], 0);
    		return 0;
    
    	case IOCTL_LED_OFF:
    		// 设置指定引脚的输出电平为1
    		s3c2410_gpio_setpin(led_table[arg], 1);
    		return 0;
    	default:
    		return -EINVAL;
    	}
    }

应用程序执行系统调用ioclt(fd, cmd, arg)时（fd是前面执行open系统调用时返回的文件句柄），s3c24xx_leds_ioctl函数将被调用。根据传入的cmd、arg参数调用s3c2410_gpio_setpin函数，来设置引脚的输出电平：输出0时点亮LED，输出1时熄灭LED。

s3c2410_gpio_setpin函数也是内核中实现的，它通过GPIO的数据寄存器来设置输出电平。

注意：应用程序执行的open、ioctl等系统调用，它们的参数和驱动程序中相应函数的参数不是一一对应的，其中经过了内核文件系统层的转换。

系统调用函数原型如下：

![](http://i.imgur.com/bvqtkid.jpg)

file_operations结构中的成员如下：

![](http://i.imgur.com/PDPQ1Nd.jpg)

可以看到，这些参数有很大一部分非常相似。

1. 系统调用open传入的参数已经被内核文件系统层处理了，在驱动程序中看不出原来的参数了。

2. 系统调用ioclt的参数个数可变，一般最多传入3个：后面两个参数与file_operations结构中ioctl成员的后两个参数对应。

3. 系统调用read传入的buf、count参数，对应file_operations结构中read成员的buf、count参数。而参数offp表示用户在文件中进行存取操作的位置，当执行完读写操作后由驱动程序设置。

4. 系统调用write与file_operations结构中write成员的参数关系，与第3点相似。

在驱动程序的最后，有如下描述信息，它们不是必须的

    /* 描述驱动程序的一些信息，不是必须的 */
    MODULE_AUTHOR("http://www.100ask.net");
    MODULE_VERSION("0.1.0");
    MODULE_DESCRIPTION("S3C2410/S3C2440 LED Driver");
    MODULE_LICENSE("GPL");

## 驱动程序编译 ##

将s3c24xx_leds.c文件放入内核drivers/char子目录下，在drivers/char/Makefile中增加下面一行：

    obj-m	+= s3c24xx_leds.o

然后在内核根目录下执行“make modules”，就可以生成模块drivers/char/s3c24xx_leds.ko。把它放到单板根文件系统的/lib/modules/2.6.22.6/目录下，就可以使用“insmod s3c24xx_leds”、“rmmod s3c24xx_leds”命令进行加载、卸载了。

## 驱动程序测试 ##

首先要编译测试程序led_test.c，它的代码很简单，关键部分如下：

    #define IOCTL_LED_ON	0
    #define IOCTL_LED_OFF	1
    int main(int argc, char **argv)
    {
    	...
    	fd = open("/dev/leds", 0);				// 打开设备
    	...
    	led_no = strtoul(argv[1], 0, 0) - 1;	// 操作哪个LED？
    
    	if (!strcmpp(argv[2], "on")) {
    		ioctl(fd, IOCTL_LED_ON, led_no);	// 点亮
    	} else if (!strcmpp(argv[2], "off")) {
    		ioctl(fd, IOCTL_LED_OFF, led_no);	// 熄灭
    	} else {
    		goto err;
    	}
    	...
    }

其中的open、ioclt最终会调用驱动程序中的s3c24xx_leds_open、s3c24xx_leds_ioctl函数。

执行“make”命令生成可执行程序led_test，将它放入单板根文件系统/usr/bin/目录下。

然后，在单板根文件系统中如下建立设备文件：

    # mknod /dev/leds c 231 0

现在就可以参照led_test的使用说明（直接运行led_test命令即可看到）操作LED了，以下两条命令点亮、熄灭LED1：

    # led_test 1 on
    # led_test 1 off
