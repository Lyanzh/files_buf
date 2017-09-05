## 背景知识 ##

### 1. LCD工作的硬件需求： ###

要使一块LCD正常的显示文字或图像，不仅需要LCD驱动器，而且还需要相应的LCD控制器。在通常情况下，生产厂商把LCD驱动器会以COF/COG的 形式与LCD玻璃基板制作在一起，而LCD控制器则是由外部的电路来实现，现在很多的MCU内部都集成了LCD控制器，如S3C2410/2440等。通 过LCD控制器就可以产生LCD驱动器所需要的控制信号来控制STN/TFT屏了。

### 2. S3C2440内部LCD控制器结构图： ###

![](http://i.imgur.com/dxiraol.png)

我们根据数据手册来描述一下这个集成在S3C2440内部的LCD控制器：

a：LCD控制器由REGBANK、LCDCDMA、TIMEGEN、VIDPRCS寄存器组成；

b：REGBANK由17个可编程的寄存器组和一块256*16的调色板内存组成，它们用来配置LCD控制器的；

c：LCDCDMA是一个专用的DMA，它能自动地把在侦内存中的视频数据传送到LCD驱动器，通过使用这个DMA通道，视频数据在不需要 CPU的干预的情况下显示在LCD屏上；

d：VIDPRCS接收来自LCDCDMA的数据，将数据转换为合适的数据格式，比如说4/8位单扫，4位双扫显示模式，然后通过数据端口 VD[23:0]传送视频数据到LCD驱动器；

e：TIMEGEN由可编程的逻辑组成，他生成LCD驱动器需要的控制信号，比如VSYNC、HSYNC、VCLK和LEND等等，而这些控制 信号又与REGBANK寄存器组中的LCDCON1/2/3/4/5的配置密切相关，通过不同的配置，TIMEGEN就能产生这些信号的不同形态，从而支 持不同的LCD驱动器(即不同的STN/TFT屏)。

### 3. 常见TFT屏工作时序分析： ###

![](http://i.imgur.com/EbmZJzA.png)

LCD提供的外部接口信号：

	VSYNC/VFRAME /STV： 垂直同步信号(TFT)/帧同步信号(STN)/SEC TFT信号；
	HSYNC/VLINE/CPV： 水平同步信号(TFT)/行同步脉 冲信号(STN)/SEC TFT信号；
	VCLK/LCD_HCLK： 象 素时钟信号(TFT/STN)/SEC TFT信号；
	VD[23:0]： LCD 像素数据输出端口(TFT/STN/SEC TFT)；
	VDEN/VM/TP： 数 据使能信号(TFT)/LCD驱动交流偏置信号(STN)/SEC TFT信号；
	LEND/STH： 行 结束信号(TFT)/SEC TFT信号；
	LCD_LPCOE： SEC TFT OE信号；
	LCD_LPCREV： SEC TFT REV信号；
	LCD_LPCREVB： SEC TFT REVB信号。

所有显示器显示图像的原理都是从上到下，从左到右的。这是什么意思呢？这么说吧，一副图像可以看做是一个矩形，由很多排列整齐的点一行一行组 成，这些点称之为像素。那么这幅图在LCD上的显示原理就是：

	A： 显示指针从矩形左上角的第一行第一个点开始，一个点一个点的在LCD上显示，在上面的时序图上用时间线表示就为VCLK，我们称之为像素时钟信号；
	B： 当显示指针一直显示到矩形的右边就结束这一行，那么这一行的动作在上面的时序图中就称之为1 Line；
	C： 接下来显示指针又回到矩形的左边从第二行开始显示，注意，显示指针在从第一行的右边回到第二行的左边是需要一定的时间的，我们称之为行切换；
	D： 如此类推，显示指针就这样一行一行的显示至矩形的右下角才把一副图显示完成。因此，这一行一行的显示在时间线上看，就是时序图上的HSYNC；
	E： 然而，LCD的显示并不是对一副图像快速的显示一下，为了持续和稳定的在LCD上显示，就需要切换到另一幅图上(另一幅图可以和上一副图一样或者不一样，目的只是为了将图像持续的显示在LCD上)。那么这一副一副的图像就称之为帧，在时序图上就表示为1 Frame，因此从时序图上可以看出1 Line只是1 Frame中的一行；
	F： 同样的，在帧与帧切换之间也是需要一定的时间的，我们称之为帧切换，那么LCD整个显示的过程在时间线上看，就可表示为时序图上的VSYNC。

上面时序图上各时钟延时参数的含义如下：(这些参数的值，LCD产生厂商会提供相应的数据手册)

	VBPD(vertical back porch)： 表示在一帧图像开始时，垂直同步信号以后的无效的行数，对应驱动中的 upper_margin；
	VFBD(vertical front porch)： 表示在一帧图像结束后，垂直同步信号以前的无效的行数，对应驱动中的lower_margin；
	VSPW(vertical sync pulse width)： 表示垂直同步脉 冲的宽度，用行数计算，对应驱动中的vsync_len；
	HBPD(horizontal back porch)： 表示从水平同步信号开始到一行的有效数据开始之间的VCLK的个数，对应驱动中的 left_margin；
	HFPD(horizontal front porth)： 表示一行的有效数据结束到下一个水平同步信号开始之间的VCLK的个数，对应驱动中的 right_margin；
	HSPW(horizontal sync pulse width)： 表示水平同步信号的宽度，用VCLK计算，对应驱动中的hsync_len；

对于以上这些参数的值将分别保存到REGBANK寄存器组中的LCDCON1/2/3/4/5寄存器中：(对寄存器的操作请查看S3c2440 数据手册LCD部分)

	LCDCON1：17 - 8位CLKVAL 
	         6 - 5位扫描模式(对于STN屏:4位单/双扫、8位单扫) 
	         4 - 1位色位模式(1BPP、8BPP、16BPP等)
	
	LCDCON2：31 - 24位VBPD 
	         23 - 14位LINEVAL 
	         13 - 6位VFPD 
	          5 - 0位VSPW
	
	LCDCON3：25 - 19位HBPD 
	         18 - 8位HOZVAL 
	          7 - 0位HFPD
	
	LCDCON4： 7 - 0位HSPW
	
	LCDCON5：

### 4. 帧缓冲(FrameBuffer)： ###

帧缓冲是Linux为显示设备提供的一个接口，它把一些显示设备描述成一个缓冲区，允许应用程序通过 FrameBuffer定义好的接口访问这些图形设备，从而不用去关心具体的硬件细节。对于帧缓冲设备而言，只要在显示缓冲区与显示点对应的区域写入颜色 值，对应的颜色就会自动的在屏幕上显示。下面来看一下在不同色位模式下缓冲区与显示点的对应关系：

![](http://i.imgur.com/X5Rf7TG.png)

## 帧缓冲(FrameBuffer)设备驱动结构 ##

帧缓冲设备为标准的字符型设备，在Linux中主设备号29，定义在/include/linux/major.h中的 FB_MAJOR，次设备号定义帧缓冲的个数，最大允许有32个FrameBuffer，定义在/include/linux/fb.h中的 FB_MAX，对应于文件系统下/dev/fb%d设备文件。

### 1. 帧缓冲设备驱动在Linux子系统中的结构如下： ###

![](http://i.imgur.com/22KwHDA.png)

我们从上面这幅图看，帧缓冲设备在Linux中也可以看做是一个完整的子系统，大体由fbmem.c和 xxxfb.c组成。向上给应用程序提供完善的设备文件操作接口(即对FrameBuffer设备进行read、write、ioctl等操作)，接口在 Linux提供的fbmem.c文件中实现；向下提供了硬件操作的接口，只是这些接口Linux并没有提供实现，因为这要根据具体的LCD控制器硬件进行设置，所以这就是我们要做的事情了(即xxxfb.c部分的实现)。

### 2. 帧缓冲相关的重要数据结构： ###

从帧缓冲设备驱动程序结构 看，该驱动主要跟fb_info结构体有关，该结构体记录了帧缓冲设备的全部信息，包括设备的设置参数、状态以及对底层硬件操作的函数指针。在Linux 中，每一个帧缓冲设备都必须对应一个fb_info，fb_info在/linux/fb.h中的定义如下：(只列出重要的一些)

	struct fb_info {
	    int node;
	    int flags;
	    struct fb_var_screeninfo var; /*LCD可变参数结构体*/
	    struct fb_fix_screeninfo fix; /*LCD固定参数结构体*/
	    struct fb_monspecs monspecs;  /*LCD显示器标准*/
	    struct work_struct queue ;    /*帧缓冲事件队列*/
	    struct fb_pixmap pixmap;      /*图像硬件mapper* /
	    struct fb_pixmap sprite;      /*光标硬件mapper*/
	    struct fb_cmap cmap;          /*当前的颜色表*/
	    struct fb_videomode * mode;   /*当前的显示模式*/
	
	# ifdef CONFIG_FB_BACKLIGHT
	     struct backlight_device * bl_dev;/*对应的背光设备 */
	    struct mutex bl_curve_mutex;
	    u8 bl_curve[ FB_BACKLIGHT_LEVELS];/*背光调整 */
	# endif
	# ifdef CONFIG_FB_DEFERRED_IO
	    struct delayed_work deferred_work;
	    struct fb_deferred_io *fbdefio;
	# endif
	
	    struct fb_ops *fbops; /* 对底层硬件操作的函数指针*/
	    struct device *device;
	    struct device *dev;   /*fb设备*/
	    int class_flag;
	# ifdef CONFIG_FB_TILEBLITTING
	    struct fb_tile_ops *tileops; /*图块Blitting* /
	# endif
	    char __iomem *screen_base;   /*虚拟基地址*/
	    unsigned long screen_size;   /*LCD IO映射的虚拟内存大小*/
	    void *pseudo_palette;        /*伪16色颜色表*/
	# define FBINFO_STATE_RUNNING    0
	# define FBINFO_STATE_SUSPENDED  1
	    u32 state;   /*LCD的挂起或恢复状态*/
	    void *fbcon_par;
	    void *par;
	};

其中，比较重要的成员有struct fb_var_screeninfo var、struct fb_fix_screeninfo fix和struct fb_ops *fbops，他们也都是结构体。下面我们一个一个的来看。

fb_var_screeninfo结构体主要记录用户可以修改的控制器的参 数，比如屏幕的分辨率和每个像素的比特数等，该结构体定义如下：

	struct fb_var_screeninfo { 
	    __u32 xres;                 /*可见屏幕一行有多少个像素点*/
	    __u32 yres;                 /*可见屏幕一列有多少个像素点*/
	    __u32 xres_virtual;         /*虚拟屏幕一行有多少个像素点*/
	    __u32 yres_virtual;         /*虚拟屏幕一列有多少个像素点*/
	    __u32 xoffset;              /*虚拟到可见屏幕之间的行偏移*/
	    __u32 yoffset;              /*虚拟到可见屏幕之间的列偏移*/
	    __u32 bits_per_pixel;       /*每个像素的位数即BPP*/
	    __u32 grayscale;            /*非0时，指的是灰度*/
	
	    struct fb_bitfield red;     /*fb缓存的R位域*/
	    struct fb_bitfield green;   /*fb缓存的G位域*/
	    struct fb_bitfield blue;    /*fb缓存的B位域*/
	    struct fb_bitfield transp;  /*透明度*/
	
	    __u32 nonstd;               /* != 0 非标准像素格式*/
	    __u32 activate;
	    __u32 height;               /*高度*/
	    __u32 width;                /*宽度*/
	    __u32 accel_flags;
	
	    /*定时：除了pixclock本身外，其他的都以像素时钟为单位*/
	    __u32 pixclock;             /*像素时钟(皮秒)*/
	    __u32 left_margin;          /*行切换，从同步到绘图之间的延迟*/
	    __u32 right_margin;         /*行切换，从绘图到同步之间的延迟*/
	    __u32 upper_margin;         /*帧切换，从同步到绘图之间的延迟*/
	    __u32 lower_margin;         /*帧切换，从绘图到同步之间的延迟*/
	    __u32 hsync_len;            /*水平同步的长度*/
	    __u32 vsync_len;            /*垂直同步的长度*/
	    __u32 sync;
	    __u32 vmode;
	    __u32 rotate;
	    __u32 reserved[5];          /*保留*/
	};

而fb_fix_screeninfo结构体又主要记录用户不可以修改的控制器的参数，比如屏幕缓冲区的物理地址和长度等，该结构体的定义如下：

	struct fb_fix_screeninfo {
	    char id[16];                 /*字符串形式的标示符 */
	    unsigned long smem_start;    /*fb缓存的开始位置 */
	    __u32 smem_len;              /*fb缓存的长度 */
	    __u32 type;                  /*看FB_TYPE_* */
	    __u32 type_aux;              /*分界*/
	    __u32 visual;                /*看FB_VISUAL_* */
	    __u16 xpanstep;              /*如果没有硬件panning就赋值为0 */
	    __u16 ypanstep;              /*如果没有硬件panning就赋值为0 */
	    __u16 ywrapstep;             /*如果没有硬件ywrap就赋值为0 */
	    __u32 line_length;           /*一行的字节数 */
	    unsigned long mmio_start;    /*内存映射IO的开始位置*/
	    __u32 mmio_len;              /*内存映射IO的长度*/
	    __u32 accel;
	    __u16 reserved[3] ;          /*保留*/
	};

fb_ops结构体是对底层硬件操作的函数指针，该结构体中定义了对硬件的操作有:(这里只列出了常用的操作)

	struct fb_ops {
	
	    struct module *owner;
	
	     //检查可变参数并进行设置
	    int (*fb_check_var) ( struct fb_var_screeninfo *var, struct fb_info *info);
	
	     //根据设置的值进行更新，使之有效
	    int ( *fb_set_par) (struct fb_info *info);
	
	     //设置颜色寄存器
	    int ( *fb_setcolreg) ( unsigned regno, unsigned red, unsigned green,
	             unsigned blue, unsigned transp, struct fb_info *info);
	
	     //显示空白
	    int (*fb_blank) (int blank, struct fb_info *info);
	
	     //矩形填充
	    void (*fb_fillrect) (struct fb_info *info, const struct fb_fillrect *rect);
	
	     //复制数据
	    void (*fb_copyarea) ( struct fb_info *info, const struct fb_copyarea *region);
	
	     //图形填充
	    void (*fb_imageblit) (struct fb_info *info, const struct fb_image *image);
	};

### 3. 帧缓冲设备作为平台设备： ###

在S3C2440中，LCD控 制器被集成在芯片的内部作为一个相对独立的单元，所以Linux把它看做是一个平台设备，故在内核代码/arch/arm/plat-s3c24xx/devs.c中定义有LCD相关的平台设备及资源，代码如下：

	/* LCD Controller */
	//LCD控制器的资源信息
	static struct resource s3c_lcd_resource[ ] = {
	    [0] = {
	        .start = S3C24XX_PA_LCD, //控制器IO端口开始地址
	        .end = S3C24XX_PA_LCD + S3C24XX_SZ_LCD - 1, //控制器IO端口结束地址 
	        .flags = IORESOURCE_MEM, //标识为 LCD控制器IO端口，在驱动中引用这个就表示引用IO端口
	    },
	    [1] = {
	        .start = IRQ_LCD , //LCD中断
	        .end = IRQ_LCD,
	        .flags = IORESOURCE_IRQ, //标识为LCD中断
	    }
	};
	
	static u64 s3c_device_lcd_dmamask = 0xffffffffUL;
	
	struct platform_device s3c_device_lcd = {
	    .name			= "s3c2410-lcd", //作为平台 设备的LCD设备名
	    .id				= -1,
	    .num_resources	= ARRAY_SIZE(s3c_lcd_resource), //资源数量
	    .resource		= s3c_lcd_resource, //引用上面 定义的资源
	    .dev = {
	    	.dma_mask = &s3c_device_lcd_dmamask,
	    	.coherent_dma_mask = 0xffffffffUL,
	    },
	};
	
	EXPORT_SYMBOL(s3c_device_lcd); //导出定义的LCD平台设备，好在mach-smdk2440.c的smdk2440_devices[]中添加到平台设备列表中

除此之外，Linux还在/arch/arm/mach-s3c2410/include/mach/fb.h中为LCD平台设备定义了一个s3c2410fb_mach_info结构体，该结构体主要是记录LCD的硬件参数信息(比如该结构体的s3c2410fb_display成员结构中 就用于记录LCD的屏幕尺寸、屏幕信息、可变的屏幕参数、LCD配置寄存器等)，这样在写驱动的时候就直接使用这个结构体。下面，我们来看一下内核是如果使用这个结构体的。在/arch/arm/mach-s3c2440/mach-smdk2440.c中定义有：

	/* LCD driver info */
	//LCD硬件的配置信息，注意这里我使用的LCD是NEC 3.5寸TFT屏，这些参数要根据具体的LCD屏进行设置
	
	static struct s3c2410fb_display smdk2440_lcd_cfg __initdata = {
	    //这个地方的设置是配置LCD寄存器5，这些宏定义在regs-lcd.h中，计 算后二进制为：111111111111，然后对照数据手册上LCDCON5的各位来看，注意是从右边开始 
	    .lcdcon5 = S3C2410_LCDCON5_FRM565 |
	               S3C2410_LCDCON5_INVVLINE |
	               S3C2410_LCDCON5_INVVFRAME |
	               S3C2410_LCDCON5_PWREN |
	               S3C2410_LCDCON5_HWSWP,
	
	    .type = S3C2410_LCDCON1_TFT, //TFT 类型
	
	    /* NEC 3.5'' */
	    .width        = 240, //屏幕宽度
	    .height       = 320, //屏幕高度
	    //以下一些参数在上面的时序图分析中讲到过,各参数的值请跟据具体的LCD屏数据手册结合上面时序分析来设定
	    .pixclock     = 100000,	//像素时钟
	    .xres         = 240,	//水平可见的有效像素
	    .yres         = 320,	//垂直可见的有效像素
	    .bpp          = 16,		//色位模式
	    .left_margin  = 19,		//行 切换，从同步到绘图之间的延迟
	    .right_margin = 36,		//行切换，从绘图到同步之间的延迟
	    .hsync_len    = 5,		//水 平同步的长度
	    .upper_margin = 1,		//帧切换，从同步到绘图之间的延迟
	    .lower_margin = 5,		//帧 切换，从绘图到同步之间的延迟
	    .vsync_len    = 1,		//垂直同步的长度
	};
	
	static struct s3c2410fb_mach_info smdk2440_fb_info __initdata = {
	    .displays        = &smdk2440_lcd_cfg, //应用上面定义的配置信息
	    .num_displays    = 1,
	    .default_display = 0,
	
	    .gpccon          = 0xaaaa555a,//将GPC0、GPC1配置成LEND和VCLK，将GPC8-15配置成VD0-7,其他配置成普通输出IO口
	    .gpccon_mask     = 0xffffffff,
	    .gpcup           = 0x0000ffff,//禁止GPIOC的上拉功能
	    .gpcup_mask      = 0xffffffff,
	    .gpdcon          = 0xaaaaaaaa,//将GPD0-15配置成VD8-23
	    .gpdcon_mask     = 0xffffffff,
	    .gpdup           = 0x0000ffff,//禁止GPIOD的上拉功能
	    .gpdup_mask      = 0xffffffff,
	
	    .lpcsel          = 0x0,//这个是三星TFT屏的参数，这里不用
	};

注意：可能有很多朋友不知道上面红色部分的参数是做什么的，其值又是怎么设置的？其实它是跟你的开发板LCD控制器密切相关的，看了下面两幅图相信 就大概知道他们是干什么用的：

![](http://i.imgur.com/I0JbYne.png)

![](http://i.imgur.com/QbICZtm.png)

上面第一幅图是开发板原理图的LCD控制器部分，第二幅图是S3c2440数据手册中IO端口C和IO端口D控制器部分。原理图中使用了 GPC8-15和GPD0-15来用做LCD控制器VD0-VD23的数据端口，又分别使用GPC0、GPC1端口用做LCD控制器的LEND和VCLK 信号，对于GPC2-7则是用做STN屏或者三星专业TFT屏的相关信号。然而，S3C2440的各个IO口并不是单一的功能，都是复用端口，要使用他们 首先要对他们进行配置。所以上面红色部分的参数就是把GPC和GPD的部分端口配置成LCD控制功能模式。

从以上讲述的内容来看，要使LCD控制器支持其他的LCD屏，重要的是根据LCD的数据手册修改以上这些参数的值。下面，我们再看一下在驱动中是如果引用 到s3c2410fb_mach_info结构体的(注意上面讲的是在内核中如何使用的)。在mach-smdk2440.c中有：

	//S3C2440初始化函数
	static void __init smdk2440_machine_init(void)
	{
	    //调用该函数将上面定义的LCD硬件信息保存到平台数据中 
	    s3c24xx_fb_set_platdata(&smdk2440_fb_info);
	    
	    s3c_i2c0_set_platdata(NULL);
	
	    platform_add_devices(smdk2440_devices, ARRAY_SIZE( smdk2440_devices));
	    smdk_machine_init();
	}

s3c24xx_fb_set_platdata定义在 plat- s3c24xx/devs.c中：

	void __init s3c24xx_fb_set_platdata( struct s3c2410fb_mach_info *pd)
	{
	    struct s3c2410fb_mach_info *npd;
	
	    npd = kmalloc(sizeof(*npd), GFP_KERNEL);
	    if (npd) {
	        memcpy(npd, pd, sizeof(*npd));
	        //这里就是将内核中定义的s3c2410fb_mach_info结构体数据保存到LCD平台数据中，所以在写驱动的时候就可以直接在平台数据中获取 s3c2410fb_mach_info结构体的数据(即LCD各种参数信息)进行操作
	        s3c_device_lcd.dev.platform_data = npd;
	    } else {
	        printk(KERN_ERR "no memory for LCD platform data/n");
	    }
	}

## 帧缓冲(FrameBuffer)设备驱动实例代码 ##

(这里不使用内核提供的平台设备代码)

	#include<linux/module.h>
	#include<linux/kernel.h>
	#include<linux/errno.h>
	#include<linux/string.h>
	#include<linux/mm.h>
	#include<linux/slab.h>
	#include<linux/delay.h>
	#include<linux/fb.h>
	#include<linux/init.h>
	#include<linux/dma-mapping.h>
	#include<linux/interrupt.h>
	#include<linux/workqueue.h>
	#include<linux/wait.h>
	#include<linux/platform_device.h>
	#include<linux/clk.h>
	
	#include<asm/io.h>
	#include<asm/uaccess.h>
	#include<asm/div64.h>
	
	#include<asm/mach/map.h>
	#include<asm/arch/regs-lcd.h>
	#include<asm/arch/regs-gpio.h>
	#include<asm/arch/fb.h>
	
	static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red, unsigned int green, unsigned int blue, unsigned int transp, struct fb_info *info);
	
	struct lcd_regs={
		unsigned long lcdcon1;
		unsigned long lcdcon2;
		unsigned long lcdcon3;
		unsigned long lcdcon4;
		unsigned long lcdcon5;
		unsigned long lcdsaddr1;
		unsigned long lcdsaddr2;
		unsigned long lcdsaddr3;
		unsigned long redlut;
		unsigned long greenlut;
		unsigned long bluelut;
		unsigned long reserved[9];
		unsigned long dithmode;
		unsigned long tpal;
		unsigned long lcdintpnd;
		unsigned long lcdsrcpnd;
		unsigned long lcdintmsk;
		unsigned long lpcsel;
	};
	
	static struct fb_ops s3c_lcdfb_ops = {
		.owner        = THIS_MODULE,
		.fb_setcolreg = s3c_lcdfb_setcolreg,
	
		/*这三个函数分别由三个模块（在drivers/vedio目录下）来具体实现，然后一起加载，它们经常用到，所以这里不用实现他们*/
		.fb_fillrect  = cfb_fillrect,/*填充矩形 */
		.fb_copyarea  = cfb_copyarea,/*拷贝一个区域*/
		.fb_imageblit = cfb_imageblit,
	};
	
	static struct fb_info *s3c_lcd;/*分配info结构体*/
	/*GPIO控制器*/
	static volatile unsigned long *gpbcon;
	static volatile unsigned long *gpbdat;
	static volatile unsigned long *gpccon;
	static volatile unsigned long *gpdcon;
	static volatile unsigned long *gpgcon;
	
	static volatile struct lcd_regs *lcd_regs;
	
	static u32 pseudo_palette[16];/*假的调色板*/
	
	/*调色板函数*/
	/*5:6:5 format*/
	static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red, unsigned int green, unsigned int blue, unsigned int transp, struct fb_info *info)
	{
		unsigned int val;
		if(regno > 16)
			return 1;
		/*用red,green,blue三原色构造出val*/
		val  = chan_to_field(red, &info->var.red);
		val |= chan_to_field(green, &info->var.green);
		val |= chan_to_field(blue, &info->var.blue);
	
		//((u32 *)(info->pseudo_palette))[regno] = val;
		pseudo_palette[regno] = val;
		return 0;
	}
	
	static int lcd_init(void)
	{
		/*1.分配一个fb_info 
		 *其中分配时fb_info里面的值默认都是0，所有下面有些参数可以不用设置默认0
		 */
		s3c_lcd = framebuffer_alloc(0,NULL);	/*其中0代表不需要额外的私有数据空间*/

		/*2.设置*/
		/*2.1设置固定的参数*/
		strcpy(s3c_lcd->fix.id, "mylcd");		/*设置fix的名称*/
		s3c_lcd->fix.smem_len = 240*320*16;		/*按具体的屏幕--设置一帧的大小*/
		s3c_lcd->fix.type     = FB_TYPE_PACKED_PIXELS;	/*默认值0*/
		s3c_lcd->fix.visual   = FB_VISUAL_TRUECOLOR;	/*TFT真彩色*/
		s3c_lcd->fix.line_lenth = 240*16/8;				/*一行的长度大小，单位byte*/

		/*2.2设置可变的参数*/
		s3c_lcd->var.xres           =240;	/*x方向的分辨率*/
		s3c_lcd->var.yres           =320;	/*y方向的分辨率*/
		s3c_lcd->var.xres_virtual   =240;	/*x方向的虚拟分辨率*/
		s3c_lcd->var.xres_virtual   =320;	/*y方向的虚拟分辨率*/
		s3c_lcd->var.bits_per_pixel	=16;	/*每个像素16位--bpp*/
	
		/*RGB--5:6:5*/
		s3c_lcd->var.red.offset   = 11;		/*第11位开始*/
		s3c_lcd->var.red.length   = 5;		/*长度5位*/
		
		s3c_lcd->var.green.offset = 5;
		s3c_lcd->var.green.length = 6;
	
		s3c_lcd->var.blue.offset  = 0;
		s3c_lcd->var.blue.length  = 5;
	
		s3c_lcd->var.activate = FB_ACTIVATE_NOW;
	
		/*2.3设置操作函数*/
		s3c_lcd->fbops = &s3c_lcdfb_ops;
	
		/*2.4其他的设置*/
		s3c_lcd->pseudo_palette = pseudo_palette;
		//s3c_lcd->screen_base  =;	/*显存的虚拟地址*/
		s3c_lcd->screen_size    = 240*320*16/8;	/*屏幕的大小，单位byte*/
	
		/*3.硬件相关的设置*/
		/*3.1配置GPIO用于LCD*/
		gpbcon = ioremap(0x56000010, 8);
		gpbdat = gpbcon + 1;/*相当于+4 byte*/
		gpccon = ioremap(0x56000020, 4);
		gpdcon = ioremap(0x56000030, 4);
		gpgcon = ioremap(0x56000060, 4);
	
		*gpccon=0xaaaaaaaa;/*GPIO管脚用于VD[7：0]，LCDVF[2：0]，VM，VFRAME，VLINE，VCLK，LEND*/
		*gpdcon=0xaaaaaaaa;/*GPIO管脚用于VD[23：8]*/
	
		*gpbcon &= ~(3);	/*GPBO设置为输出引脚*/
		*gpbcon |= 1;
		*gpbdat &= ~1;		/*输出低电平*/
	
		*gpgcon |= (3<<8);	/*GPG4用作LCD_PWREN*/
	
		/*3.2根据LCD手册设置LCD控制器，比如VCLK的频率等*/
		lcd_regs = ioremap(0x4D000000, sizeof(struct lcd_regs));
	
		/*bit[17:8]:VCLK=HCLK/[(CLKVAL+1)*2],LCD手册P14
		 *			10MHz=100MHz/[CLKVAL+1]*2]
		 *          10MHz是我们2440手册里的100ns得出的（1us->1MHz）
		 *			100MHz是我们主机的频率
		 *			CLKVAL=4
		 *bit[6:5]:	ob11,TFT LCD
		 *bit[4:1]:	ob1100,16 bpp for TFT
		 *bit[0]:  0=Disable the video output and the LCD control signal.
		 */
		lcd_regs->lcdcon1 =(4<<8) | (3<<5) | (0x0c<<1);
	
		/*垂直方向的时间参数
		 *bit[31:24]:VBPD,VSYNC之后再过多长时间才能发出第1行数据
		 *			LCD手册  T0-T2-T1=4
		 *			VBPD=3
		 *bit[23:14]:多少行，320，所以LINEVAL=320-1=319
		 *bit[13:6]	:VFPD,发出最后一行数据之后，再过多长时间才发出VSYNC
		 *			LCD手册T2-T5=322-320=2，所以VFPD=2-1=1
		 *bit[5:0]	: VSPW,VSYNC信号的脉冲宽度，LCD手册T1=1，所以VSPW=1-1=0
		 */
		lcd_regs->lcdcon2 =(3<<24) | (319<<14) | (1<<6) | (0<<0);
	
		/*水平方向的时间参数
		 *bit[25:19]:HBPD,HSYNC之后再过多长时间才能发出第1行数据
		 *			 LCD手册  T6-T7-T8=17
		 *			 HBPD=16
		 *bit[18:8]:多少列，240，所以HOZVAL=240-1=239
		 *bit[7:0]:HFPD,发出最后一行里最后一个像素数据之后，再过多长时间才能发出HSYNC
		 *			LCD手册T8-T11=251-240=11，所以HFPD=11-1=10
		 */
		 lcd_regs->lcdcon3=(16<<19) | (239<<9) | (10<<0);
		
		/*水平方向的同步信号
		 *bit[7:0]:HSPW,HSYNC信号的脉冲宽度，LCD手册T7=5，所以HSPW=5-1=4
		 */
		lcd_regs->lcdcon4 = 4;
	
		/*信号的极性
		*bit[11]: 1=5:6:5 format
		*bit[10]: 0=The video data is fetched at VCLK falling edge
		*bit[9]:  1=HSYNC信号要反转，即低电平有效
		*bit[8]:  1=VSYNC信号要反转，即低电平有效
		*bit[6]:  0=VDEN不用反转
		*bit[3]:  0=PWREN输出0
		*下面两位控制在framebuffer中[0:31]->[P1,P2]还是[p2,p1]
		*bit[1]:  0=BSWP
		*bit[0]:  1=HWSWP 2440手册P413
		*很明显是[p1,p2]
		*/
		lcd_regs->lcdcon5= (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);
		
		/*3.3分配显存（framebuffer）,并把地址告诉LCD控制器*/
	
		/*申请一块连续的内存，返回虚拟地址*/
		s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->fix.smem_start, GFP_KERNEL);
	
		/*lcdsaddr1的bit[29:0]对应A[30:1],最低1位不要,所以右移一位--最高两位不需要--清0*/
		lcd_regs->lcdsaddr1 = (s3c_lcd->fix.smem_start >> 1) & ~(3<<30);
	
		/*lcsaddr2的bit[20:0]对应A[21:1],所以最低一位不需要，右移一位，其他位清0*/
		lcd_regs->lcdsaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1)& 0x1fffff;
	
		lcd_regs->lcdsaddr3 = (240*16/16);/*一行的长度（单位2字节）*/
	
		//s3c_lcd->fix.smem_start = xxx;/*显存的物理地址*/
	
		/*启动LCD*/
		lcd_regs->lcdcon1 |= (1<<0);/*使能LCD控制器*/
		lcd_regs->lcdcon5 |= (1<<3);/*使能LCD本身*/
		*gpbdat |= 1;				/*输出高电平，使能背光*/
	
	
		/*4.注册*/
		register_framebuffer(s3c_lcd);
		return 0;
	}
	
	static void lcd_exit(void)
	{
		unrigster_framebuffer(s3c_lcd);
		lcd_regs->lcdcon1 &= ~(1<<0);	/*关闭LCD本身*/
		*gpbdat &= ~1;					/*关闭背光*/
		dma_free_writecombine(NULL, s3c_lcd->fix.smem_len, s3c_lcd->screen_base, s3c_lcd->fix.smem_start);
		iounmap(lcd_regs);
		iounmap(gpbcon);
		iounmap(gpccon);
		iounmap(gpdcon);
		iounmap(gpgcon);
		framebuffer_release(s3c_lcd);
	}
	
	module_init(lcd_init);
	module_exit(lcd_exit);
	
	MODULE_LICENSE("GPL");

## S3C2410调色板技术概述 ##

1. 调色板的概念

	在计算机图像技术中，一个像素的颜色是由它的R，G，B分量表示的，每个分量又经过量化，一个 像素总的量化级数就是这个显示系统的颜色深度。量化级数越高，可以表示的颜色也就越多，最终的图像也就越逼真。当量化级数达到16位以上时，被称为真彩 色。但是，量化级数越高，就需要越高的数据宽度，给处理器带来的负担也就越重；量化级数在8位以下时，所能表达的颜色又太少，不能够满足用户特定的需求。

	为了解决这个问题，可以采取调色板技术。所谓调色板，就是在低颜色深度的模式下，在有限的像素 值与RGB颜色之间建立对应关系的一个线性表。比如说，从所有的16位彩色中抽取一定数量的颜色，编制索引。当需要使用某种彩色时，不需要对这种颜色的RGB分量进行描述，只需要引用它的索引号，就可以使用户选取自己需要的颜色。索引号的编码长度远远小于RGB分量的编码长度，因此在彩色显示的同时，也 大大减轻了系统的负担。

	以256色调色板为例，调色板中存储256种颜色的RGB值，每种颜色的RGB值是16位。用 这256种颜色编制索引时，从OOH～FFH只需要8位数据宽度，而每个索引所对应的颜色却是16位宽度的颜色信息。在一些对色彩种类要求不高的场合，如 仪表终端、信息终端等，调色板技术便巧妙地解决了数据宽度与颜色深度之间的矛盾。

2. S3C2410中的调色板

	ARM9核的S3C2410芯片可通过内置的LCD控制器来实现对LCD显示的控制。以TFTLCD为例，S3C2410芯片的LCD控制器可以对TFTLCD提供1位、2位、4位、8位调色板彩色显示和16位、24位真彩色显示，并支持多 种不同的屏幕尺寸。

	S3C2410的调色板其实是256个16位的存储单元，每个单元中存储有16位的颜色值。根 据16位颜色数据中，RGB分量所占位数的不同，调色板还可以采取5：6：5(R：G：B)和5：5：5：1(R：G：B：1)两种格式。当采用5：6：5(R：G：B)格式时，它的调色板如表1所示。

	![](http://i.imgur.com/F9PA3RP.jpg)

	表1中，第一列为颜色索引，中间三列是R，G，B三个颜色分量对应的数据位，分别是5位、6位 和5位，最后一列是对应颜色条目的物理地址。当采用5：5：5：1(R：G：B：1)格式时，R，G，B三个颜色分量的数据位长度都是5位，最低位为1。

	用户编程时，应首先对调色板进行初始化处理(可由操作系统提供的驱动程序来完成)，赋予256色调色板相应的颜色值；在进行图像编程时，可以将图像对象赋予所需的颜色索引值。程序运行时，由芯片的LCD控制器查找调色板，按相应的值进行输出。S3C2410芯片图像数据输出端口VD[23：O]有24位，当使用不同的色彩深度时，这24位数据可以表示一个或多个点的颜色信息。

3. 调色板颜色的选择

	调色板中颜色的选择可以由用户任意定义，但为了编程方便，颜色的选取应遵循一定的规律。例如在Windows编程中，系统保留了20种颜色。另外，在Web编程中，也定义了216种Web安全色，这些颜色可以尽量保留。S3C2410调色板在嵌入式Linux系统下的使用ARM实现图像显示时，由LCD控制器将存储系统中的视频缓冲内容以及各种控制信号传送到外部LCD驱动器，然后由LCD驱动 器实现图像数据的显示。实际应用中，常通过驱动程序由操作系统对寄存器、调色板进行配置。以Linux2.4内核为例，对调色板的配置是在驱动程序S3C2410fb.c中完成的。

	在一些公司Linux源码包的S3C2410fb.c文件中，并没有对调色板进行配置，因此在8位以下的显示设置下。LCD不能正常工作。若需要使用调色板，必须对此文件进行修改。

	3.1 驱动程序的修改

	查S3C2410数据手册，调色板的物理起始地址为0x4d000400，应先将调色板的物理 地址映射到内核中的虚拟地址，然后对其进行赋值。具体步骤如下：

	(1)在S3C2410.h文件中添加：

		#define MYPAL(Nb) __REG(Ox4d000400+(Nb)*4)

	其作用是实现物理地址到虚拟地址的映射。

	(2)在S3C24lOfb.h文件，通过下列语句定义256种颜色。

		static const u_short my_color[256]= {0x0000，0x8000，…};

	数组中的每个16位二进制数表示一种颜色，RGB分量采用的是5：6：5格式。

	(3)在S3C2410fb.c文件的S3C2410fb-activate_var(…)函 数中，通过下列语句对这256个调色板进行赋值。

		for (i = 0; i < 256; i++)
		{
			MYPAL(i) = my_color[i];
		}


	(4)另外，注意改变LCD控制寄存器LCDCON1的BPPMODE值，设定为需要的颜色深度。

	(5)重新编译内核，烧写内核。

	3.2 应用程序的编写

	当S3C2410用于嵌入式Linux操作系统时，其图形功能一般是依靠帧缓存(Framebuffer)实现的。屏幕上的每个点都被映射成一段线性内存空间，通过应用程序改变这段内存的值，就可以改变屏幕的颜色。当色深在16位以 上时，用户直接指定颜色的RGB分量；当色深在8位以下时，用户应当指定颜色在调色板中的索引值。

	当使用MiniGUI等嵌入式图形系统时，只需要将界面元素的颜色值设为所需颜色的索引值即 可。例如：

		WinElementColors[i] = 142；

	就是将WinElementColors[i]的颜色设置为索引号为142的调色板颜色。