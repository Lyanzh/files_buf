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
					dev->features |=  NETIF_F_IP_CSUM|NETIF_F_SG;
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

