#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/init.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <linux/if_ether.h>	/* For the statistics structure. */
#include <linux/if_arp.h>	/* For ARPHRD_ETHER */
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/percpu.h>

#if 0
struct net_device s3c_vnet_dev = {
	.name	 		= "vnet0",
	//.get_stats		= &get_stats,
	//.mtu			= (16 * 1024) + 20 + 20 + 12,
	.hard_start_xmit	= loopback_xmit,
	//.hard_header		= eth_header,
	//.hard_header_cache	= eth_header_cache,
	//.header_cache_update	= eth_header_cache_update,
	//.hard_header_len	= ETH_HLEN,	/* 14	*/
	//.addr_len		= ETH_ALEN,	/* 6	*/
	//.tx_queue_len		= 0,
	.type			= ARPHRD_LOOPBACK,	/* 0x0001*/
	//.rebuild_header		= eth_rebuild_header,
	.flags			= IFF_NOARP,
	.features 		= NETIF_F_NO_CSUM,
	//.ethtool_ops		= &loopback_ethtool_ops,
};
#endif

static struct net_device *s3c_vnet_dev;

static void s3c_vnet_emulator_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
	/* 参考LDD3 */
	unsigned char *type;
	struct iphdr *ih;
	__be32 *saddr, *daddr, tmp;
	unsigned char tmp_dev_addr[ETH_ALEN];
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

static int s3c_vnet_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	static int cnt = 0;
	printk("virt_net_send_packet cnt = %d\n", ++cnt);

	/* 对于真实的网卡, 把skb里的数据通过网卡发送出去 */
	netif_stop_queue(dev); /* 停止该网卡的队列 */
	/* ...... */           /* 把skb的数据写入网卡 */

	/* 构造一个假的sk_buff,上报 */
	s3c_vnet_emulator_rx_packet(skb, dev);

	dev_kfree_skb(skb);		/* 释放skb */
	netif_wake_queue(dev);	/* 数据全部发送出去后,唤醒网卡的队列 */
	
	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;
	return 0;
}

static int __init s3c_vnet_init(void)
{
	s3c_vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);

	s3c_vnet_dev->hard_start_xmit = s3c_vnet_send_packet;
	s3c_vnet_dev->dev_addr[0] = 0x08;
	s3c_vnet_dev->dev_addr[1] = 0x89;
	s3c_vnet_dev->dev_addr[2] = 0x89;
	s3c_vnet_dev->dev_addr[3] = 0x89;
	s3c_vnet_dev->dev_addr[4] = 0x89;
	s3c_vnet_dev->dev_addr[5] = 0x11;

	s3c_vnet_dev->flags |= IFF_NOARP;
	s3c_vnet_dev->features |= NETIF_F_NO_CSUM;
	
	register_netdev(s3c_vnet_dev);

	return 0;
}

static void __exit s3c_vnet_exit(void)
{
	unregister_netdev(s3c_vnet_dev);
	free_netdev(s3c_vnet_dev);
}

module_init(s3c_vnet_init);
module_exit(s3c_vnet_exit);
MODULE_LICENSE("GPL");

