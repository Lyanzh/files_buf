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

static int s3c_vnet_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	static int cnt = 0;
	printk("virt_net_send_packet cnt = %d\n", ++cnt);
	
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
