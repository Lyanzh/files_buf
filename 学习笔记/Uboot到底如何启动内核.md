1. uboot启动内核的代码缩减如下：

Uboot 1.16/lib_arm/board.c中start_armboot()函数调用/common/main.c中main_loop()函数，在main_loop()中有uboot启动内核的代码：

    s = getenv ("bootcmd");
    debug ("### main_loop: bootcmd=\"%s\"\n", s ? s :"<UNDEFINED>");
    if (bootdelay >= 0 && s && !abortboot (bootdelay))
    {
    	run_command(s, 0);
    }

2.假设bootcmd = nand read.jffs2 0x30007FC0 kernel; bootm 0x30007FC0

<1> nand read.jffs2 0x30007FC0 kernel

从nand读出内核：

从哪里读？  ：kernel分区

读到哪里去？：0x30007FC0

何为分区？ ：简单的说就是将nand划分为几个区域，一般如下：
 
    bootloader->params->kernel->root
 
这些分区在/include/configs/100ask24x0.h中写死了：
 
    #define MTDPARTS_DEFAULT"mtdparts=nandflash0:256k@0(bootloader)," \
               "128k(params)," \
               "2m(kernel)," \
               "-(root)"

进入uboot执行mtd ,可以查看已有分区:

![](http://i.imgur.com/8TeO4Wn.png)

上面的nand read.jffs2 0x30007FC0 kernel等价于：

    nand read.jffs2 0x30007FC0 0x00060000 0x00200000

注：read.jffs2并不是指定特定格式，仅表示不需要块/页对齐,所以kernel的分区大小可以随意定。

<2> bootm  0x30007FC0

关键函数do_bootm()
 
flash上存的内核：uImage

uImage = 头部+真正的内核

头部的定义如下：

    typedef struct image_header {
        uint32_t       ih_magic;
        uint32_t       ih_hcrc;
        uint32_t       ih_time;
        uint32_t       ih_size;
        uint32_t       ih_load;
        uint32_t       ih_ep;
        uint32_t       ih_dcrc;
        uint8_t        ih_os;
        uint8_t        ih_arch; 
        uint8_t        ih_type;
        uint8_t        ih_comp;
        uint8_t        ih_name[IH_NMLEN];
    } image_header_t;
 
我们需要关心：

        uint32_t       ih_load;
        uint32_t       ih_ep;

ih_load是加载地址，即内核运行时应该位于的地方

ih_ep是入口地址，即内核的入口地址
 
这与uboot类似，uboot的加载地址是TEXT_BASE = 0x33F80000；入口地址是start.S中的_start。
 
从nand读出来的内核可以放在ram中的任意地方，如0x31000000，0x32000000等等，只要它不破坏uboot所占用的内存空间就可以
 
既然设定好了加载地址和入口地址, 为什么内核还能随意放？

因为uImage有一个头部！头部里有加载地址和入口地址，当我们用bootm xxx时，do_bootm先去读uImage的头部以获取该uImage的加载地址和入口地址，当发现该uImage目前所处的内存地址不等于它的加载地址时，会将uImage移动到它的加载地址上，代码中体现如下：

uboot 1.16/common/cmd_bootm.c中的bootm_load_os()函数

    case IH_COMP_NONE:：
    if (load != image_start)
    {
    	memmove_wd((void *)load, (void *)image_start, image_len, CHUNKSZ);
    }

另外，当内核正好处于头部指定的加载地址，便不用uboot的do_bootm函数来帮我们搬运内核了，可以缩短启动时间。这就是为什么我们一般都下载uImage到0x30007FC0的原因。
 
内核加载地址是0x30008000，而头部的大小64个字节，将内核拷贝到0x30007FC0，加上头部的64个字节，内核正好位于0x30008000处。

总结bootm做了什么：

1. 读取头部

2. 将内核移动到加载地址

3. 启动内核

具体如何启动内核？

使用在/lib_arm/bootm.c定义的do_bootm_linux()，我们已经知道入口地址，只需跳到入口地址就可以启动linux内核了，在这之前需要做一件事 -- uboot传递参数(启动参数)给内核。

启动代码在do_bootm_linux()函数：

    void (*theKernel)(int zero, int arch,uint params);  //定义函数指针theKernel
    theKernel = (void (*)(int, int, uint))images->ep;    //先是将入口地址赋值给theKernel
    theKernel (0, bd->bi_arch_number, bd->bi_boot_params); //然后是调用thekernel，以0，bd->bi_arch_number，bd->bi_boot_params为参数
 
下面分析这三个参数：

1.  0 -- 相当于mov ，r0， #0

2.  bd->bi_arch_number ：uboot机器码，这个在/board/100ask24x0.c设置： gd->bd->bi_arch_number = MACH_TYPE_S3C2440，MACH_TYPE_S3C2440在/arch/arm/asm/mach-types.h定义：362, 内核机器码和uboot机器码必须一致才能启动内核。

3. bd->bi_boot_parmas -- 启动参数地址，也是在/board/100ask24x0.c设置： gd->bd->bi_boot_params = 0x30000100;

启动参数(tag)在哪里设置？

在lib_arm/armlinux.c：

    setup_start_tag (bd);
    setup_revision_tag (parmas);
    setup_memory_tags (bd);
    setup_commandline_tag (bd, commandline);
    setup_initrd_tag (bd, images->rd_start, images->rd_end);
    setup_videolfb_tag ((gd_t *) gd);
    setup_end_tag (bd);

每一个启动参数对应一个tag结构体，所谓的设置传递参数其实就是初始化这些tag的值，想了解这个结构体以及这些tag的值是如何设置的请看<<嵌入式Linux应用开发完全手册>>uboot章节
 
我们重点关注setup_start_tag(bd)函数：

    static void setup_start_tag (bd_t *bd)
    {
        params = (struct tag *) bd->bi_boot_params;  
        params->hdr.tag = ATAG_CORE;
        params->hdr.size = tag_size(tag_core);
        params->u.core.flags = 0;
        params->u.core.pagesize = 0;
        params->u.core.rootdev = 0;
        params = tag_next (params);
    }

再看setup_commandline_tag (bd , commandline)：

    static void setup_commandline_tag (bd_t *bd, char*commandline)
    {
       // commandline就是我们的bootargs
        char *p;
        if (!commandline)
        	return;
        for (p = commandline; *p == ' '; p++);
        if (*p == '\0')
        	return;
        params->hdr.tag = ATAG_CMDLINE;
        params->hdr.size =
        	(sizeof(struct tag_header) + strlen (p) + 1 + 4) >> 2;
        strcpy (params->u.cmdline.cmdline, p);
        params = tag_next (params);
    }

内核启动时会读取这些tag(参数)并跳转启动。