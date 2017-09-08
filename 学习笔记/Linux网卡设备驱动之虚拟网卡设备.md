## Linux网卡设备驱动之虚拟网卡设备 ##

### 框架分析 ###

网络协议分可为七层等等。我们只关心硬件部分，就是底层的网卡部分的内容。应用通过socket就可以传输数据。

![](https://i.imgur.com/4jhu6vr.png)

下面，我们说一下实际的网络包究竟经过怎样一个流程从用户空间发送到驱动，再到实际的硬件设备的，或者怎样由实际的设备接收之后，经由设备驱动层传递到用户空间的。

![](https://i.imgur.com/geesjEl.png)

- 接收过程，如上如，网络上的数据包到达网卡后，网卡产生中断，然后设备驱动层收到中断后，开始进行网络包的接收，接收完之后调用一个netif_rx函数交给网络协议层（层次结构上图一），然后就是一层一层的网上传到用户空间了。
- 发送过程，从用户空间过来的数据包，经过层层穿越之后，到达网络协议层，然后调用一个dev_queue_xmit()函数之后就不管了，剩下的交给驱动层经过处理后，使用函数hard_start_xmit()函数发送，然后硬件上网卡开始发送数据包了。

1）、网络协议接口层向网络层协议提供提供统一的数据包收发接口，不论上层协议为ARP还是IP，都通过dev_queue_xmit()函数发送数据，并通过netif_rx()函数接受数据。这一层的存在使得上层协议独立于具体的设备。  
2）、网络设备接口层向协议接口层提供统一的用于描述具体网络设备属性和操作的结构体net_device，该结构体是设备驱动功能层中各函数的容器。实际上，网络设备接口层从宏观上规划了具体操作硬件的设备驱动功能层的结构。  
3）、设备驱动功能层各函数是网络设备接口层net_device数据结构的具体成员，是驱使网络设备硬件完成相应动作的程序，他通过hard_start_xmit()函数启动发送操作，并通过网络设备上的中断触发接受操作。  
4）、网络设备与媒介层是完成数据包发送和接受的物理实体，包括网络适配器和具体的传输媒介，网络适配器被驱动功能层中的函数物理上驱动。对于Linux系统而言，网络设备和媒介都可以是虚拟的。  
5）、网络协议接口层：主要进行数据包的收发。

### 编写网卡驱动代码的一般过程 ###

1. 分配`net_device`
2. 设置  
	a. 提供发包函数：`hard_start_xmit`  
	b. 提供收包的功能
3. 注册：`register_netdev`
4. 硬件相关操作

#### 参考：Cs89x0.c (drivers\net) ####

入口函数 `init_module`

	int __init init_module(void)
	{
		struct net_device*dev =alloc_etherdev(sizeof(struct net_local));
	    /* 设置默认MAC地址，
	     * MAC地址可以由CS8900A外接的EEPROM设定(有些单板没接EEPROM)，
	     * 或者启动系统后使用ifconfig修改
	     */
	    dev->dev_addr[0] = 0x08;
	    dev->dev_addr[1] = 0x89;
	    dev->dev_addr[2] = 0x89;
	    dev->dev_addr[3] = 0x89;
	    dev->dev_addr[4] = 0x89;
	    dev->dev_addr[5] = 0x89; 
		ret = cs89x0_probe1(dev, io, 1);
	}

`cs89x0_probe1` 函数

	cs89x0_probe1(struct net_device *dev, int ioaddr, int modular)
	{
		dev->open  = net_open;
		dev->stop  = net_close;
		dev->tx_timeout= net_timeout;
		dev->watchdog_timeo= HZ;
		dev->hard_start_xmit= net_send_packet;
		dev->get_stats= net_get_stats;
		dev->set_multicast_list = set_multicast_list;
		dev->set_mac_address 
		= set_mac_address;
		retval = register_netdev(dev);
	}

中断函数 `net_interrupt`

	static irqreturn_t net_interrupt(int irq, void *dev_id)
	{
		net_rx(dev);
	}

	net_rx(struct net_device *dev)
	{
		struct sk_buff *skb;
		skb = dev_alloc_skb(length + 2);
		netif_rx(skb);
	}

总结

我们可以看出协议与硬件之间就是通过这函数 `hard_start_xmit` 和 `netif_rx` 来通讯的。收发的内容是一个 `sk_buff` 结构体来描述的。

### 虚拟网卡设备驱动编写 ###

1. 分配一个 `net_device` 结构体  
2. 设置:  
2.1 发包函数: `hard_start_xmit`  
2.2 收到数据时(在中断处理函数里)用 `netif_rx` 上报数据  
2.3 其他设置  
3. 注册: `register_netdevice`  

![](https://i.imgur.com/hrEYzjG.png)

主要代码

	static struct net_device *vnet_dev;
	
	static int virt_net_init(void)
	{
		/* 1. 分配一个net_device结构体 */
		vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);;  /* alloc_etherdev */
	
		/* 2. 设置 */
		vnet_dev->hard_start_xmit = virt_net_send_packet;
	
		/* 设置MAC地址 */
	    vnet_dev->dev_addr[0] = 0x08;
	    vnet_dev->dev_addr[1] = 0x89;
	    vnet_dev->dev_addr[2] = 0x89;
	    vnet_dev->dev_addr[3] = 0x89;
	    vnet_dev->dev_addr[4] = 0x89;
	    vnet_dev->dev_addr[5] = 0x11;
	
	    /* 设置下面两项才能ping通 */
		vnet_dev->flags           |= IFF_NOARP;
		vnet_dev->features        |= NETIF_F_NO_CSUM;	
	
		/* 3. 注册 */
		//register_netdevice(vnet_dev);
		register_netdev(vnet_dev);
		
		return 0;
	}
	
	static void virt_net_exit(void)
	{
		unregister_netdev(vnet_dev);
		free_netdev(vnet_dev);
	}

发包函数：

	static int virt_net_send_packet(struct sk_buff *skb, struct net_device *dev)
	{
		static int cnt = 0;
		printk("virt_net_send_packet cnt = %d\n", ++cnt);
	
		/* 对于真实的网卡, 把skb里的数据通过网卡发送出去 */
		netif_stop_queue(dev); /* 停止该网卡的队列 */
	    /* ...... */           /* 把skb的数据写入网卡 */
	
		/* 构造一个假的sk_buff,上报 */
		emulator_rx_packet(skb, dev);
	
		dev_kfree_skb (skb);   /* 释放skb */
		netif_wake_queue(dev); /* 数据全部发送出去后,唤醒网卡的队列 */
	
		/* 更新统计信息 */
		dev->stats.tx_packets++;
		dev->stats.tx_bytes += skb->len;
		
		return 0;
	}

其中 `emulator_rx_packet(skb, dev);`

	static void emulator_rx_packet(struct sk_buff *skb, struct net_device *dev)
	{
		/* 参考LDD3 */
		unsigned char *type;
		struct iphdr *ih;
		__be32 *saddr, *daddr, tmp;
		unsigned char	tmp_dev_addr[ETH_ALEN];
		struct ethhdr *ethhdr;
		
		struct sk_buff *rx_skb;
			
		// 从硬件读出/保存数据
		/* 对调"源/目的"的mac地址 */
		ethhdr = (struct ethhdr *)skb->data;
		memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
		memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
		memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);
	
		/* 对调"源/目的"的ip地址 */    
		ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
		saddr = &ih->saddr;
		daddr = &ih->daddr;
	
		tmp = *saddr;
		*saddr = *daddr;
		*daddr = tmp;
		
		//((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
		//((u8 *)daddr)[2] ^= 1;
		type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
		//printk("tx package type = %02x\n", *type);
		// 修改类型, 原来0x8表示ping
		*type = 0; /* 0表示reply */
		
		ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
		ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
		
		// 构造一个sk_buff
		rx_skb = dev_alloc_skb(skb->len + 2);
		skb_reserve(rx_skb, 2); /* align IP on 16B boundary */	
		memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);
	
		/* Write metadata, and then pass to the receive level */
		rx_skb->dev = dev;
		rx_skb->protocol = eth_type_trans(rx_skb, dev);
		rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += skb->len;
	
		// 提交sk_buff
		netif_rx(rx_skb);
	}

测试

1. >> insmod virt_net.ko
2. >> ifconfig vnet0 3.3.3.3
   >> ifconfig // 查看
3. >> ping 3.3.3.3
   >> ping 3.3.3.4