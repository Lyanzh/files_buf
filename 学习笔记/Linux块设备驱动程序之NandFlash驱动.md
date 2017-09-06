### nandflash原理图： ###

![](https://i.imgur.com/pkb89IG.jpg)

- LDATA0——LDATA7：既传输数据，也传输地址，还传输命令。  
- RnB：读写nandflash的状态标志位，0表示读写完成了，1表示还在忙。  
- CLE：当CLE为高电平时表示传输的是命令  
- ALE：当ALE为高电平时表示传输的是地址，当CLE和ALE都为低电平时表示传输的是数据，这就区分开了命令、地址和数据。  
- nFWE：写使能  
- nFRE：读使能  
- nFCE：片选信号  

### 操作nandflash ###

1. 发命令：  
	（1）选中芯片：nFCE=0  
	（2）CLE设为高电平  
	（3）在DATA0——DATA1上输出命令值：00h  
	（4）发出一个写脉冲：nFWE下降沿命令被锁存

2. 写地址：  
	（1）选中芯片：nFCE=0  
	（2）ALE设为高电平  
	（3）在DATA0——DATA1上输出地址值：这个地址是自己规定的  
	（4）发出写脉冲

3. 发命令：  
	（1）选中芯片：nFCE=0  
	（2）CLE设为高电平  
	（3）在DATA0——DATA1上输出命令值：00h  
	（4）发出一个写脉冲：nFWE下降沿命令被锁存  

4. 读数据：  
	（1）选中芯片  
	（2）发出读脉冲  
	（3）读DATA0~DATA7的数据  

cpu里面集成了nandflash控制器，我们只需要将命令、地址、数据写入相应的寄存器里面，就能够发送出去，那些时序的东西，nandflash控制器会帮我们做好的。那么我们的读操作就变得很简单了：  
（1）选中nandflash，这通过配置NFCONT寄存器来实现  
（2）00h写入命令寄存器：NFCMMD=0x00  
（3）把地址写入地址寄存器：NFADDR，有效位是8位  
（4）30h写入命令寄存器：NFCMMD=30  
（5）从数据寄存器读数据：val=NFDATA 

### 用UBOOT来体验NAND FLASH的操作： ###

1. 读ID

	                               S3C2440                   u-boot 
	    选中                     NFCONT的bit1设为0      md.l 0x4E000004 1;
	                                                   mw.l 0x4E000004 1
	    发出命令0x90             NFCMMD=0x90            mw.b 0x4E000008 0x90 
	    发出地址0x00             NFADDR=0x00            mw.b 0x4E00000C 0x00
	    读数据得到0xEC           val=NFDATA             md.b 0x4E000010 1
	    读数据得到device code    val=NFDATA             md.b 0x4E000010 1
	    退出读ID的状态           NFCMMD=0xff            mw.b 0x4E000008 0xff

	具体操作：  
	（1）进入uboot  
	（2）以4字节显示0x4E000004地址处的数据：md.l 0x4E000004 1，这里的1表示显示1个4字节的数据。  
	（3）将 NFCONT 倒数第二位写为0：mw.l 0x4E000004 1  
	（4）发出命令0x90：mw.b 0x4E000008 0x90  
	（5）发出地址0x00：mw.b 0x4E00000C 0x00  
	（6）读数据得到0xEC：md.b 0x4E000010 1，1表示读一个数据  
	（7）读数据得到device code---->0xda：md.b 0x4E000010 1  
	（8）退出读ID的状态：mw.b 0x4E000008 0xff  

2. 读内容: 读0地址的数据

	使用UBOOT命令:

	    >>nand dump 0
	    	Page 00000000 dump:
	    	17 00 00 ea 14 f0 9f e5  14 f0 9f e5 14 f0 9f e5

	上面`nand dump 0`是uboot里面封装好的一个命令，那我们能否用一些最基本的命令来实现这个读操作呢？答案是肯定的：

	                               S3C2440                   u-boot 
	    选中                      NFCONT的bit1设为0      md.l 0x4E000004 1;
	                                                    mw.l 0x4E000004  1
	    发出命令0x00              NFCMMD=0x00            mw.b 0x4E000008 0x00 
	    发出地址0x00              NFADDR=0x00            mw.b 0x4E00000C 0x00
	    发出地址0x00              NFADDR=0x00            mw.b 0x4E00000C 0x00
	    发出地址0x00              NFADDR=0x00            mw.b 0x4E00000C 0x00
	    发出地址0x00              NFADDR=0x00            mw.b 0x4E00000C 0x00
	    发出地址0x00              NFADDR=0x00            mw.b 0x4E00000C 0x00
	    发出命令0x30              NFCMMD=0x30            mw.b 0x4E000008 0x30 
	    读数据得到0x17            val=NFDATA             md.b 0x4E000010 1
	    读数据得到0x00            val=NFDATA             md.b 0x4E000010 1
	    读数据得到0x00            val=NFDATA             md.b 0x4E000010 1
	    读数据得到0xea            val=NFDATA             md.b 0x4E000010 1
	    退出读状态                NFCMMD=0xff            mw.b 0x4E000008 0xff


### nandflash驱动 ###

1. 定义nand_chip、mtd_info两个结构体

		static struct mtd_info *s3c_mtd;
		static struct nand_chip *s3c_nand_chip;

	nand_chip 结构体：是给nand_scan函数用的，而nand_scan函数提供了选中nand、发出命令、发出地址、发出数据、读取数据、判断状态等功能，所以nand_chip结构体上必须定义一系列实现上面功能能的函数，包括选中函数，负责发地址与命令的函数，以及判断状态的函数，最重要的就是io读取的虚拟地址。

	mtd_info结构体：MTD（Memory Technology Device）即内存技术设在linux内核中，引入mtd层为NOR Flash和NAND Flash设备提供统一的接口，将文件系统于底层Flash存储设备进行了隔离。

	MTD设备可以分为四层，从上到下依次为：设备节点层，MTD设备层，MTD原始设备层，Flash硬件驱动层。

	**Flash硬件驱动层**：负责对Flash硬件的读、写和擦除操作。MTD设备的NAND flash芯片的驱动在drivers/mtd/nand目录下，nor flash芯片驱动位于drivers/mtd/chips目录下。

	**MTD原始设备层**：用于描述MTD原始设备的数据结构体是mtd_info ，它定义了大量的关于MTD的数据和操作函数，其中mtdcore.c:实现原始设备接口的相关实现，mtdpart.c：实现mtd分区接口相关实现。

	**MTD设备层**： 基于MTD原始设备，linux系统可以定义出MTD的块设备(主设备号31)和字符设备(设备号90)，其中mtdchar.c实现mtd字符设备接口相关实现，mtdblock.c用于实现块设备接口相关实现。

	**设备节点层**：通过mknode在/dev子目录下建立MTD块设备节点,通过此设备节点即可访问MTD字符设备和块设备。

2. 增加ECC校验码

	前面的程序是可以直接用的，但是加载驱动时会报ECC校验错误。

	在flash设备中，每一页都有64b是不参与编址的，这一块是不参与统一编址OBB(out of bank)。

	原因：nand flash内存中，数据很容易发生位反转，为了防止数据发生错误，引入了ECC校验。

	解决方案：写一页数据时，这一页的数据生成ECC校验码，然后把ECC校验码写入OBB中。

	读取数据时：首先读取page，读OOB里的ECC，根据page的内容实时计算ECC，看是否与OOB中的ECC相同，若是相同则说明数据没有错误，否则通过ECC校验码也可以算出是哪一位发生错误。

	实现方法：可以设置为软件实现或者硬件实现，只要在nand_chip中的ECC结构体的mode中设置为NAND_ECC_SOFT，就是开启了软件ECC校验。

		nand_chip->ecc.mode = NAND_ECC_SOFT;/*使能ECC校验码 enable ECC */

3. 增加分区挂接

	要增加分区的话，首先要定义mtd_partition结构体，里面定义了分区的名字，起始地址，以及分区的大小等参数。

		/* 5.add_mtd_partitions 添加分区 */
		//如果想整块flash只作为一个分区，使用add_mtd_device就够了
		//add_mtd_device(s3c_mtd);
		//如果要创建多个分区的话，那么就要使用add_mtd_partitions
		add_mtd_partitions(s3c_nand_mtd, s3c_nand_part, 4);

4. 实例代码

		/* 参考 
		 * drivers\mtd\nand\s3c2410.c
		 * drivers\mtd\nand\at91_nand.c
		 */
		
		#include <linux/module.h>
		#include <linux/types.h>
		#include <linux/init.h>
		#include <linux/kernel.h>
		#include <linux/string.h>
		#include <linux/ioport.h>
		#include <linux/platform_device.h>
		#include <linux/delay.h>
		#include <linux/err.h>
		#include <linux/slab.h>
		#include <linux/clk.h>
		 
		#include <linux/mtd/mtd.h>
		#include <linux/mtd/nand.h>
		#include <linux/mtd/nand_ecc.h>
		#include <linux/mtd/partitions.h>
		 
		#include <asm/io.h>
		 
		#include <asm/arch/regs-nand.h>
		#include <asm/arch/nand.h>
		
		struct s3c_nand_regs {
			unsigned long nfconf  ;
			unsigned long nfcont  ;
			unsigned long nfcmd   ;
			unsigned long nfaddr  ;
			unsigned long nfdata  ;
			unsigned long nfeccd0 ;
			unsigned long nfeccd1 ;
			unsigned long nfeccd  ;
			unsigned long nfstat  ;
			unsigned long nfestat0;
			unsigned long nfestat1;
			unsigned long nfmecc0 ;
			unsigned long nfmecc1 ;
			unsigned long nfsecc  ;
			unsigned long nfsblk  ;
			unsigned long nfeblk  ;
		};
		
		
		static struct nand_chip *s3c_nand;
		static struct mtd_info *s3c_mtd;
		static struct s3c_nand_regs *s3c_nand_regs;
		
		static struct mtd_partition s3c_nand_parts[] = {
			[0] = {
		        .name   = "bootloader",
		        .size   = 0x00040000,
				.offset	= 0,
			},
			[1] = {
		        .name   = "params",
		        .offset = MTDPART_OFS_APPEND,
		        .size   = 0x00020000,
			},
			[2] = {
		        .name   = "kernel",
		        .offset = MTDPART_OFS_APPEND,
		        .size   = 0x00200000,
			},
			[3] = {
		        .name   = "root",
		        .offset = MTDPART_OFS_APPEND,
		        .size   = MTDPART_SIZ_FULL,
			}
		};
		
		static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
		{
			if (chipnr == -1)
			{
				/* 取消选中: NFCONT[1]设为1 */
				s3c_nand_regs->nfcont |= (1<<1);		
			}
			else
			{
				/* 选中: NFCONT[1]设为0 */
				s3c_nand_regs->nfcont &= ~(1<<1);
			}
		}
		
		static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
		{
			if (ctrl & NAND_CLE)
			{
				/* 发命令: NFCMMD=dat */
				s3c_nand_regs->nfcmd = dat;
			}
			else
			{
				/* 发地址: NFADDR=dat */
				s3c_nand_regs->nfaddr = dat;
			}
		}
		
		static int s3c2440_dev_ready(struct mtd_info *mtd)
		{
			return (s3c_nand_regs->nfstat & (1<<0));
		}
		
		
		static int s3c_nand_init(void)
		{
			struct clk *clk;
			
			/* 1. 分配一个nand_chip结构体 */
			s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
		
			s3c_nand_regs = ioremap(0x4E000000, sizeof(struct s3c_nand_regs));
			
			/* 2. 设置nand_chip */
			/* 设置nand_chip是给nand_scan函数使用的, 如果不知道怎么设置, 先看nand_scan怎么使用 
			 * 它应该提供:选中,发命令,发地址,发数据,读数据,判断状态的功能
			 */
			s3c_nand->select_chip = s3c2440_select_chip;
			s3c_nand->cmd_ctrl    = s3c2440_cmd_ctrl;
			s3c_nand->IO_ADDR_R   = &s3c_nand_regs->nfdata;
			s3c_nand->IO_ADDR_W   = &s3c_nand_regs->nfdata;
			s3c_nand->dev_ready   = s3c2440_dev_ready;
			s3c_nand->ecc.mode    = NAND_ECC_SOFT;
			
			/* 3. 硬件相关的设置: 根据NAND FLASH的手册设置时间参数 */
			/* 使能NAND FLASH控制器的时钟 */
			clk = clk_get(NULL, "nand");
			clk_enable(clk);              /* CLKCON'bit[4] */
			
			/* HCLK=100MHz
			 * TACLS:  发出CLE/ALE之后多长时间才发出nWE信号, 从NAND手册可知CLE/ALE与nWE可以同时发出,所以TACLS=0
			 * TWRPH0: nWE的脉冲宽度, HCLK x ( TWRPH0 + 1 ), 从NAND手册可知它要>=12ns, 所以TWRPH0>=1
			 * TWRPH1: nWE变为高电平后多长时间CLE/ALE才能变为低电平, 从NAND手册可知它要>=5ns, 所以TWRPH1>=0
			 */
		#define TACLS    0
		#define TWRPH0   1
		#define TWRPH1   0
			s3c_nand_regs->nfconf = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);
		
			/* NFCONT: 
			 * BIT1-设为1, 取消片选 
			 * BIT0-设为1, 使能NAND FLASH控制器
			 */
			s3c_nand_regs->nfcont = (1<<1) | (1<<0);
			
			/* 4. 使用: nand_scan */
			s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
			s3c_mtd->owner = THIS_MODULE;
			s3c_mtd->priv  = s3c_nand;
			
			nand_scan(s3c_mtd, 1);  /* 识别NAND FLASH, 构造mtd_info */
			
			/* 5. add_mtd_partitions */
			add_mtd_partitions(s3c_mtd, s3c_nand_parts, 4);
			
			//add_mtd_device(s3c_mtd);
			return 0;
		}
		
		static void s3c_nand_exit(void)
		{
			del_mtd_partitions(s3c_mtd);
			kfree(s3c_mtd);
			iounmap(s3c_nand_regs);
			kfree(s3c_nand);
		}
		
		module_init(s3c_nand_init);
		module_exit(s3c_nand_exit);
		
		MODULE_LICENSE("GPL");

