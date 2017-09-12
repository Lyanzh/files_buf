## Linux网卡设备驱动之移植DM9000C网卡驱动程序 ##

## 移植分析 ##

协议类的驱动，我们的主要工作往往是将现有的驱动和我们的硬件所匹配起来。协议类的函数往往已经成型不需要我们去修改和编写。比如发包函数:hard_start_xmit函数和netif_rx上报函数都不需要我们编写。  

网络驱动是针对很多硬件编写出来的，我们使用的是什么硬件CPU,比如ARM9，以及我们使用的系统版本。  

我们只需要修改驱动，告诉驱动现在的硬件情况是怎么样的，基地址是多少，中断引脚是哪个、设置下内存管理器以满足时序等等。这也是网络驱动移植的简单之处。

1. DM9000C  
	一般一款网卡芯片，出厂的时候会有厂家自带的驱动程序代码，我们只需要在厂家的基础上去修改移植成适合我们CPU和系统的驱动程序。

2. 厂家自带驱动程序分析  

		static int __init dm9dev9000c_init(void)
		{
		#if defined(CONFIG_ARCH_S3C2410)
			iobase = (int)ioremap(0x20000000, 4096);
			irq    = IRQ_EINT7; 
		#endif
			switch(mode) {
				case DM9KS_10MHD:
				case DM9KS_100MHD:
				case DM9KS_10MFD:
				case DM9KS_100MFD:
					media_mode = mode;
					break;
				default:
					media_mode = DM9KS_AUTO;
			}
			dmfe_dev = dmfe_probe();
			if(IS_ERR(dmfe_dev))
				return PTR_ERR(dmfe_dev);
			return 0;
		}

		struct net_device * __init dmfe_probe(void)
		{
			...
			err = dmfe_probe1(dev);
			...
			err = register_netdev(dev);
			...
			return dev;
		}

	显然dmfe_probe1函数在做注册前的设置：

		int __init dmfe_probe1(struct net_device *dev)
		{
			...
			do {
				...
				/* driver system function */				
				dev->base_addr 	= iobase;
				dev->irq 		= irq;
				dev->open 		= &dmfe_open;
				dev->hard_start_xmit 	= &dmfe_start_xmit;
				dev->watchdog_timeo	= 5*HZ;	
				dev->tx_timeout		= dmfe_timeout;
				dev->stop 			= &dmfe_stop;
				dev->get_stats 		= &dmfe_get_stats;
				dev->set_multicast_list = &dm9000_hash_table;
				dev->do_ioctl 		= &dmfe_do_ioctl;
				dev->ethtool_ops = &dmfe_ethtool_ops;
				//dev->features |=  NETIF_F_IP_CSUM;
				dev->features   |=  NETIF_F_IP_CSUM|NETIF_F_SG;
				...
			}while(!dm9000_found && iobase <= DM9KS_MAX_IO);
		
			...
		}


	其中：

		dev->base_addr	= iobase;
		dev->irq 		= irq;
		dev->open 		= &dmfe_open;


	`dmfe_open` 函数：

		static int dmfe_open(struct net_device *dev)
		{
			...
		    if (request_irq(dev->irq, &dmfe_interrupt, IRQF_SHARED|IRQF_TRIGGER_RISING, dev->name, dev))
				return -EAGAIN;
			...
		}

	iobase、irq需要我们根据硬件CPU去设置。

	![](https://i.imgur.com/F0mNRCQ.png)
	![](https://i.imgur.com/dFYpmAY.png)
	![](https://i.imgur.com/atAiZ6b.png)
	![](https://i.imgur.com/Uhy4Aaz.png)

	中断号：EINT7;
	片选：CPU发出的地址位于0x20000000-0x28000000之间

3. 移植基本思路

	修改一下几项 找出相异性

	![](https://i.imgur.com/aZVojBj.png)

## 移植示例 ##

1. 注释掉  
	![](https://i.imgur.com/FmFRrm7.png)
	![](https://i.imgur.com/vRex7dl.png)

2. 修改入口出口函数名称  
	![](https://i.imgur.com/vGqPHVs.png)

3. iobase需要设置，入口函数设置最好：  
	![](https://i.imgur.com/zpOfcCV.png)

4. 注释掉版本核对  
	![](https://i.imgur.com/Fsuezv8.png)

5. 查看中断号  
	![](https://i.imgur.com/8gWNB42.png)
	![](https://i.imgur.com/2AZVPLh.png)

6. 入口函数设置中断号  
	![](https://i.imgur.com/UMIuXzZ.png)

	设置中断触发方式-上升沿触发  
	![](https://i.imgur.com/euhIe2x.png)

7. 编译  
	![](https://i.imgur.com/vSuPdoh.png)
	![](https://i.imgur.com/5hReC8l.png)

8. 加入缺少的头文件  
	![](https://i.imgur.com/xNlA0D4.png)
	![](https://i.imgur.com/impOazH.png)

9. 类型转换  
	![](https://i.imgur.com/6Xdnxvo.png)
	![](https://i.imgur.com/HO4lLgM.png)

10. 基本移植好，但是时序没有和CPU进行匹配，不同硬件，涉及到文件收发，一定有时序要求。因此要设置内存控制器中关于网卡部分的寄存器以满足时间参数。

	主要代码：
	
		/* 设置S3C2440的memory controller */
		bwscon   = ioremap(0x48000000, 4);
		bankcon4 = ioremap(0x48000014, 4);
		
		/* DW4[17:16]: 01-16bit
		 * WS4[18]   : 0-WAIT disable
		 * ST4[19]   : 0 = Not using UB/LB (The pins are dedicated nWBE[3:0])
		*/
		val = *bwscon;
		val &= ~(0xf<<16);
		val |= (1<<16);
		*bwscon = val;
		
		/*
		 * Tacs[14:13]: 发出片选信号之前,多长时间内要先发出地址信号
		 *              DM9000C的片选信号和CMD信号可以同时发出,
		 *              所以它设为0
		 * Tcos[12:11]: 发出片选信号之后,多长时间才能发出读信号nOE
		 *              DM9000C的T1>=0ns, 
		 *              所以它设为0
		 * Tacc[10:8] : 读写信号的脉冲长度, 
		 *              DM9000C的T2>=10ns, 
		 *              所以它设为1, 表示2个hclk周期,hclk=100MHz,就是20ns
		 * Tcoh[7:6]  : 当读信号nOE变为高电平后,片选信号还要维持多长时间
		 *              DM9000C进行写操作时, nWE变为高电平之后, 数据线上的数据还要维持最少3ns
		 *              DM9000C进行读操作时, nOE变为高电平之后, 数据线上的数据在6ns之内会消失
		 *              我们取一个宽松值: 让片选信号在nOE放为高电平后,再维持10ns, 
		 *              所以设为01
		 * Tcah[5:4]  : 当片选信号变为高电平后, 地址信号还要维持多长时间
		 *              DM9000C的片选信号和CMD信号可以同时出现,同时消失
		 *              所以设为0
		 * PMC[1:0]   : 00-正常模式
		 *
		 */
		*bankcon4 = (1<<8)|(1<<6);/* 对于DM9000C可以设Tacc为1, 对于DM9000E,Tacc要设大一点,比如最大值7  */
		//*bankcon4 = (7<<8)|(1<<6);  /* MINI2440使用DM9000E,Tacc要设大一点 */
		
		iounmap(bwscon);
		iounmap(bankcon4);

11. 测试DM9000C驱动程序:  
	（1）把dm9dev9000c.c放到内核的drivers/net目录下  
	（2）修改drivers/net/Makefile  
	把 `obj-$(CONFIG_DM9000) += dm9000.o`  
	改为 `obj-$(CONFIG_DM9000) += dm9dev9000c.o`  
	（3）make uImage  
		使用新内核启动  
	（4）使用NFS启动或  
		`>>ifconfig eth0 192.168.1.17`  
		`>>ping 192.168.1.1`