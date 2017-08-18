摘自：http://forum.ubuntu.org.cn/viewtopic.php?t=162276

# ubuntu 网卡指定 eth0及其它 #

有时候虽只有一个网络接口，但网络连接（logical name:）是eth1 或为eth2甚至为eth更大的数字，这点也很讨厌，影响一些程序的默认使用

解决：

Step1：

    sudo lshw -C network

或者

    ifconfig -a

得到网卡的mac地址(serial)，注意不要和1394的serial搞混，比如我的是：00:88:88:ff:12:61

Step2：

备份:

    sudo mv /etc/udev/rules.d/70-persistent-net.rules /etc/udev/rules.d/70-persistent-net.rules.bak

Step3：

接着编辑：

    sudo gedit /etc/udev/rules.d/70-persistent-net.rules

修改物理mac与对应的网卡名：

    SUBSYSTEM=="net", ACTION=="add", DRIVERS=="?*", ATTR{address}=="00:88:88:ff:12:61", ATTR{type}=="1", KERNEL=="eth*", NAME="eth0"

Step4：

接着可以配制网卡了

手工编辑配制文件：

    sudo gedit /etc/network/interfaces

编辑:

    auto lo
    auto eth0
    iface lo inet loopback
    iface eth0 inet static
    address 192.168.0.8
    netmask 255.255.255.0
    gateway 192.168.0.254
    broadcast 192.168.0.255
    mtu 1300
    #wireless-key 3311220088
    #wireless-essid ubuntu

上面为静态ip的设定，对应内容分别是ip地址，子网掩码，网关，广播地址，mtu值，最后为无线网卡相关的key 和ssid，需要可以打开。

如果是dhcp,则可写为：

    auto lo
    auto eth0
    iface lo inet loopback
    iface eth0 inet dhcp
    #wireless-key 3311220088
    #wireless-essid ubuntu

Step5：

最后重启网络服务

    sudo /etc/init.d/networking restart