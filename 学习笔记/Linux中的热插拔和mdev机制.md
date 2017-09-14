mdev 是 busybox 自带的一个简化版的 udev ，作用是在系统启动和热插拔或动态加载驱动程序时，自动产生驱动程序所需的节点文件，在文件系统中的 /dev 目录下的设备节点都是由 mdev 创建的 mdev 扫描 /sys/class 和 /sys/block 中所有的类设备目录，如果在目录中含有名为"dev"的文件，且文件中包含的是设备号，则 mdev 就利用这些信息为这个设备在 /dev 下创建设备节点。

用法：  

1. 执行mdev前要挂载 /sys  

		mount -t tmpfs mdev /dev
		mount -t sysfs sysfs /sys

2. 命令内核在增删设备时执行/sbin/mdev，使设备节点会被创建和删除

		echo /sbin/mdev > /proc/sys/kernel/hotplug

3. 设置mdev，让它在系统启动时创建所有的设备节点

		mdev -s

关于热插拔：  
需要内核中支持hotplug  
编写mdev配置文件：/etc/mdev.conf  
该文件的作用是：mdev在找到匹配设备时自动执行自定义命令  

/etc/mdev.conf 格式为：  

	<device regex> <uid>:<gid> <octal permissions> [<@$*><cmd>]

- @ 创建节点后执行的  
- $ 删除节点前执行的  
- * 创建后和删除前都运行的  

如自动挂载U盘和SD卡脚本：  

	sd[a-z][0-9] 0:0 0660 @/etc/hotplug/usb/udisk_insert
	sd[a-z] 0:0 0660 $/etc/hotplug/usb/udisk_remove
	mmcblk[0-9]p[0-9] 0:0 0660 @/etc/hotplug/sd/sd_insert
	mmcblk[0-9] 0:0 0660 $/etc/hotplug/sd/sd_remove

当检测到类似sda1这样的设备时，执行/etc/hotplug/usb里的脚本，脚本的内容就是挂载和卸载U盘  
当检测到类似mmcblk0p1这样的设备时，执行/etc/hotplug/sd里的脚本，脚本的内容就是挂载和卸载SD卡  

mdev是busybox中的一个udev管理程序的一个精简版，他也可以实现设备节点的自动创建和设备的自动挂载，只是在实现的过程中有点差异，在发生热插拔时间的时候，mdev是被hotplug直接调用，这时mdev通过环境变量中的 ACTION 和 DEVPATH，来确定此次热插拔事件的动作以及影响了/sys中的那个目录。接着会看看这个目录中是否有“dev”的属性文件，如果有就利用这些信息为这个设备在/dev 下创建设备节点文件。

下面是如何让我们的系统支持mdev

1. 在使用busybox制作根文件系统的时候，选择支持mdev

		Linux System Utilities ---> 
		[*] mdev 
		[*] Support /etc/mdev.conf
		[*] Support command execution at device addition/removal

2. 在文件系统添加如下内容

		vim /etc/init.d/rcS
		mount -t tmpfs mdev /dev 
		mount -t sysfs sysfs /sys
		mkdir /dev/pts
		mount -t devpts devpts /dev/pts
		echo /sbin/mdev>/proc/sys/kernel/hotplug
		mdev –s

	这些语句的添加在mdev的手册中可以找到。

3. 添加对热插拔事件的响应，实现U盘和SD卡的自动挂载。

		vim /etc/mdev.conf
		mmcblk[0-9]p[0-9] 0:0 666 @ /etc/sd_card_inserting
		mmcblk[0-9] 0:0 666 $ /etc/sd_card_removing
		sd[a-z] [0-9] 0:0 666 @ /etc/usb/usb_inserting
		sd[a-z] 0:0 666 $ /etc/usb/usb_removing

	红色部分，是一个脚本，脚本内容可以根据我们的需要定制，可以实现挂载，卸载或其他一些功能。

	如下是自动挂载和卸载的脚本：

		/etc/sd_card_inserting
		#!/bin/sh
		mount -t vfat /dev/mmcblk0p1 /mnt/sd
		/etc/sd_card_removing
		#!/bin/sh
		sync
		umount /mnt/sd