# 移植Busybox #
## Busybox概述 ##
- 与一般的GNU工具集动辄几MB的体积相比，动态连接的Busybox只有几百KB，即使静态连接也只有1MB左右。Busybox按模块进行设计，可以很容易地加入、去除某些命令，或增减命令的某些选项。

- **在创建一个最小的根文件系统时，使用Busybox的话，只需要在/dev目录下创建必要的设备节点、在/etc目录下创建一些配置文件就可以了**，如果Busybox使用动态连接，还要在/lib目录下包含库文件。

- Busybox支持uClibc库和glibc库，对Linux 2.2.x之后的内核支持良好。

## init进程介绍及用户程序启动过程 ##

- init进程是由内核启动的第一个（也是惟一的一个）用户进程（进程ID为1），它根据配置文件决定启动哪些程序，比如执行某些脚本、启动shell、运行用户指定的程序等。init进程是后续所有进程的发起者，比如init进程启动/bin/sh程序后，才能够在控制台上输入各种命令。

- **init进程的执行程序通常是/sbin/init，上面讲述的init进程的作用只不过是/sbin/init这个程序的功能。我们完全可以编写自己的/sbin/init程序，或者传入命令行参数“init=xxxxx”指定某个程序作为init进程运行。**

- 一般而言，在Linux系统有两种init程序：BSD init和System V init。BSD和System V是两种版本的UNIX系统。这两种init程序各有优缺点，现在大多Linux的发行版本使用System V init。但是在嵌入式领域，通常使用Busybox集成的init程序。

### 内核如何启动init进程 ###

内核启动的最后一步就是启动init进程，代码在init/main.c文件中，如下所示：

![](http://i.imgur.com/ED5BLc0.jpg)
![](http://i.imgur.com/tdnGOKF.jpg)

其中的run_init_process函数使用它的参数所指定的程序来创建一个用户进程。需要注意，一旦run_init_process函数创建进程成功，它将不会返回。

内核启动init进程的过程如下:

1. **打开标准输入、标准输出、标准错误设备**

	Linux中最先打开的3个文件分别称为标准输入（stdin）、标准输出（stdout）、标准错误（stderr），它们对应的文件描述符分别为0、1、2。所谓标准输入就是在程序中使用scanf()、fscanf(stdin,…)获取数据时，从哪个文件（设备）读取数据；标准输出、标准错误都是输出设备，前者对应printf()、fprintf(stdout,…)，后者对应fprintf(stderr,…)。

	第756行尝试打开/dev/console设备文件，如果成功，它就是init进程标准输入设备。

	第759、760将文件描述符0复制给文件描述符1、2，所以标准输入、标准输出、标准错误都对应同一个文件（设备）。
	
	在移植Linux内核时，如果发现打印出“Warning: unable to open an initial console.”，其原因大多是：根文件系统虽然被正确挂接了，但是里面的内容不正确，要么没有/dev/console这个文件，要么它没有对应的设备。

2. **如果ramdisk_execute_command变量指定了要运行的程序，启动它**
	
	ramdisk_execute_command的取值（代码也在init/main.c中）分3种情况:
	
	   ①　如果命令行参数中指定了“rdinit=…”，则ramdisk_execute_command等于这个参数指定的程序。
	
	   ②　否则，如果/init程序存在，ramdisk_execute_command就等于“/init”。
	
	   ③　否则，ramdisk_execute_command为空。
	
	本书所用的命令行没有设定“rdinit=…”，根文件系统中也没有/init程序，所以ramdisk_execute_command为空，第763～765这几行的代码不执行。

3. 如果execute_command变量指定了要运行的程序，启动它

	如果命令行参数中指定了“init=…”，则execute_command等于这个参数指定的程序，否则为空。
	
	本书所用的命令行没有设定“init=…”，所以第775～777行代码不执行。

4. **依次尝试执行/sbin/init、/etc/init、/bin/init、/bin/sh**

	第779行执行/sbin/init程序，这个程序在我们的根文件系统中是存在的，所以init进程所用的程序就是/sbin/init。从此系统的控制权交给/sbin/init，不再返回init_post函数中。

	run_init_process函数也在init/main.c中，代码如下：

	![](http://i.imgur.com/V2jRfb0.jpg)

	所以执行/sbin/init程序时，它的环境参数为“"HOME＝/", "TERM＝linux"”。

## Busybox init进程的启动过程 ##

Busybox init程序对应的代码在init/init.c文件中，下面以busybox-1.7.0为例进行讲解。先概述其流程，再结合一个/etc/inittab文件讲述init进程的启动过程。

### Busybox init程序流程 ###

流程图如图所示，其中与构建根文件系统关系密切的是控制台的初始化、对inittab文件的解释及执行。

![Busybox init程序流程](http://i.imgur.com/3TyL3Lu.jpg)

- **内核启动init进程时已经打开“/dev/console”设备作为控制台，一般情况下Busybox init程序就使用/dev/console。但是如果内核启动init进程的同时设置了环境变量CONSOLE或console，则使用环境变量所指定的设备。在Busybox init程序中，还会检查这个设备是否可以打开，如果不能打开则使用“/dev/null”。**

- **Busybox init进程只是作为其他进程的发起者和控制者，并不需要控制台与用户交互，所以init进程会把它关掉**，系统启动后运行命令“ls /proc/1/fd/”可以看到该目录为空。**init进程创建其他子进程时，如果没有在/etc/inittab中指明它的控制台，则使用前面确定的控制台**。

- /etc/inittab文件的相关文档和示例代码都在Busybox的examples/inittab文件中。

**如果存在/etc/inittab文件，Busybox init程序解析它，然后按照它的指示创建各种子进程；否则使用默认的配置创建子进程。**

/etc/inittab文件中每个条目用来定义一个子进程，并确定它的启动方法，格式如下：

    <id>:<runlevels>:<action>:<process>

例如：

    ttySAC0::askfirst:-/bin/sh

对于Busybox init程序，上述各个字段作用如下：

- id：表示这个子进程要使用的控制台（即标准输入、标准输出、标准错误设备）。如果省略，则使用与init进程一样的控制台。

- runlevels：对于Busybox init程序，这个字段没有意义，可以省略。

- action：表示init进程如何控制这个子进程，有如表所示的8种取值。

![](http://i.imgur.com/LfD02Dk.jpg)  

- process：要执行的程序，它可以是可执行程序，也可以是脚本。

如果<procss>字段前有“-”字符，这个程序被称为“交互的”。

在/etc/inittab文件的控制下，init进程的行为总结如下：  

1. 在系统启动前期，init进程首先启动<action>为sysinit、wait、once的3类子进程。  

2. 在系统正常运行期间，init进程首先启动<action>为respawn、askfirst的两类子进程，并监视它们，发现某个子进程退出时重新启动它。  

3. 在系统退出时，执行<action>为shutdown、restart、ctrlaltdel的3类子进程（之一或全部）。

如果根文件系统中没有/etc/inittab文件，Busybox init程序将使用如下默认的inittab条目:

![](http://i.imgur.com/oUpKR5v.jpg)

### /etc/inittab实例 ###

仿照Busybox的examples/inittab文件，创建一个inittab文件，内容如下：

![](http://i.imgur.com/h5wkwAI.jpg)

## 编译/安装Busybox ##

从http://www.busybox.net/downloads/下载busybox-1.7.0.tar.bz2。解压得到busybox-1.7.0目录，里面就是所有的源码。

Busybox集合了几百个命令，在一般系统中并不需要全部使用。可以通过配置Busybox来选择这些命令、定制某些命令的功能（选项）、指定Busybox的连接方法（动态连接还是静态连接）、指定Busybox的安装路径。

### 配置Busybox ###

在busybox-1.7.0目录下执行“make menuconfig”命令即可进入配置界面。Busybox将所有配置项分类存放，下表列出了这些类别，其中的“说明”是针对嵌入式系统而言的。

![Busybox配置选项分类](http://i.imgur.com/7SsEQX5.jpg)
![Busybox配置选项分类](http://i.imgur.com/sfjxpcc.jpg)

本节使用默认配置，执行“make menuconfig”后退出、保存配置即可。

### 编译和安装Busybox ###

编译之前，先修改Busybox根目录的Makefile，使用交叉编译器。

修改前：

![](http://i.imgur.com/L0FAbIz.jpg)

修改后：

![](http://i.imgur.com/7yYyb4m.jpg)

然后可执行“make”命令编译Busybox。

最后是安装，执行“make CONFIG_PREFIX＝dir_path install”就可以将Busybox安装在dir_name指定的目录下。执行以下命令在/work/nfs_root/fs_mini目录下安装Busybox。

    make CONFIG_PREFIX=/work/nfs_root/fs_mini install

一切完成后，将在/work/nfs_root/fs_mini目录下生成如下文件、目录。

![](http://i.imgur.com/7mLe1B1.jpg)

其中linuxrc和上面分析的/sbin/init程序功能完全一样；其他目录下是各种命令，不过它们都是到/bin/busybox的符号连接，比如/work/nfs_root/fs_mini/sbin目录下：

![](http://i.imgur.com/yi0KLa9.jpg)

除bin/busybox外，其他文件都是到bin/busybox的符号连接。busybox是所有命令的集合体，这些符号连接文件可以直接运行。比如在开发板上，运行“ls”命令和“busybox ls”命令是一样的。

## 使用glibc库 ##

在制作交叉编译工具链时，已经生成了glibc库，可以直接使用它来构建根文件系统。

### glibc库的组成 ###

glibc库的位置是/work/tools/gcc-3.4.5- glibc-2.3.6/arm-linux/lib。

这个目录下的文件并非都属于glibc库，比如crt1.o、libstdc＋＋.a等文件是GCC工具本身生成的。本书不区分它们的来源，统一处理。

里面的目录、文件可以分为8类：

	①　加载器ld-2.3.6.so、ld-linux.so.2
	
	动态程序启动前，它们被用来加载动态库。
	
	②　目标文件（.o）
	
	比如crt1.o、crti.o、crtn.o、gcrt1.o、Mcrt1.o、Scrt1.o等。在生成应用程序时，这些文件像一般的目标文件一样被连接。
	
	③　静态库文件（.a）
	
	比如静态数学库libm.a、静态C＋＋库libstdc＋＋.a等，编译静态程序时会连接它们。
	
	④　动态库文件（.so、.so.[0-9]*）
	
	比如动态数学库libm.so、动态C＋＋库libstdc＋＋.so等，它们可能是一个链接文件。编译动态库时会用到这些文件，但是不会连接它们，运行时才连接。
	
	⑤　libtool库文件（.la）
	
	在连接库文件时，这些文件会被用到，比如它们列出了当前库文件所依赖的其他库文件。程序运行时无需这些文件。
	
	⑥　gconv目录
	
	里面是有头字符集的动态库，比如ISO8859-1.so、GB18030.so等。
	
	⑦　ldscripts目录
	
	里面是各种连接脚本，在编译应用程序时，它们被用于指定程序的运行地址、各段的位置等。
	
	⑧　其他目录及文件

### 安装glibc库 ###

在开发板上只需要加载器和动态库，假设要构建的根文件系统目录为/work/nfs_root/fs_mini，操作如下:

    mkdir -p /work/nfs_root/fs_mint/lib
    cd /work/tools/gcc-3.4.5-glibc-2.3.6/arm-linux/lib
    cp *.so* /work/nfs_root/fs_mini/lib -d

上面复制的库文件不是每个都会被用到，可以根据应用程序对库的依赖关系保留需要用到的。通过ldd命令可以查看一个程序会用到哪些库，主机自带的ldd命令不能查看交叉编译出来的程序，有以下两种替代方法。

①　如果有uClibc-0.9.28的代码，可以进入utils子目录生成ldd.host工具

    cd uClibc-0.9.28/utils
    make ldd.host

然后将生成的ldd.host放到主机/usr/local/bin目录下即可使用。比如对于动态连接的Busybox，它的库依赖关系如下：

![](http://i.imgur.com/t2BZbX6.jpg)

这表示Busybox要使用的库文件有libcrypt.so.1、libm.so.6、libc.so.6，加载器为/lib/ld-linux.so.2（实际上在交叉工具链目录下，加载器为ld-linux.so.2）。上面的“not found”表示主机上没有这个文件，这没关系，开发板的根文件系统上有就行。


②　可以使用以下命令：

    arm-linux-readelf -a "your binary" | grep "Shared"

比如对于动态连接的Busybox，它的库依赖关系如下：

![](http://i.imgur.com/smSvt31.jpg)

里面没有列出加载器，构造根文件系统时，它也要复制进去。

## 构建根文件系统 ##

上面两节在介绍了如何安装Busbybox、C库，建立了bin/、sbin/、usr/bin/、usr/sbin/、lib/等目录，最小根文件系统的大部分目录、文件已经建好。本节介绍剩下的部分。

### 构建etc目录 ###

init进程根据/etc/inittab文件来创建其他子进程，比如调用脚本文件配置IP地址、挂接其他文件系统，最后启动shell等。

etc目录下的内容取决于要运行的程序，本节只需要创建3个文件：etc/inittab、etc/init.d/rcS、etc/fstab。

#### 创建etc/inittab文件 ####

仿照Busybox的examples/inittab文件，在/work/nfs_root/fs_mini/etc目录下创建一个inittab文件，内容如下:

    # /etc/inittab
    ::sysinit:/etc/init.d/rcS
    ttySAC0::askfirst:-/bin/sh
    ::ctrlaltdel:/sbin/reboot
    ::shutdown:/bin/umount -a -r

#### 创建etc/init.d/rcS文件 ####

这是一个脚本文件，可以在里面添加想自动执行命令。以下命令配置IP地址、挂接/etc/fstab指定的文件系统。

    #!/bin/sh
    ifconfig eth0 192.168.1.17
    mount -a

第一行表示这是一个脚本文件，运行时使用/bin/sh解析。

第二行用来配置IP地址。

第三行挂接/etc/fstab文件指定的所有文件系统。

最后，还要改变它的属性，使它能够执行:

    chmod +x etc/init.d/rcS

#### 创建etc/fstab文件 ####

内容如下，表示执行“mount -a”命令后将挂接proc、tmpfs文件系统

    # device    mount-point    type    options    dump    fsck order
    proc         /proc         proc    defaults    0       0
    tmpfs        /tmp          tmpfs   defaults    0       0

/etc/fstab文件被用来定义文件系统的“静态信息”，这些信息被用来控制mount命令的行为。文件中各字段意义如下:

①　device：要挂接的设备

比如/dev/hda2、/dev/mtdblock1等设备文件；也可以是其他格式，比如对于proc文件系统这个字段没有意义，可以是任意值；对于NFS文件系统，这个字段为 host:dir。

②　mount-point：挂接点

③　type：文件系统类型

比如proc、jffs2、yaffs、ext2、nfs等；也可以是auto，表示自动检测文件系统类型。

④　options：挂接参数，以逗号隔开

/etc/fstab的作用不仅仅是用来控制“mount -a”的行为，即使是一般的mount命令也受它控制。除与文件系统类型相关的参数外，常用的有以下几种取值

![/etc/fstab参数字段常用的取值](http://i.imgur.com/FxSR3Cd.jpg)

⑤　dump和fsck order：用来决定控制dump、fsck程序的行为。

dump是一个用来备份文件的程序，fsck是一个用来检查磁盘的程序。要想了解更多信息，请阅读它们的man手册。

dump程序根据dump字段的值来决定这个文件系统是否需要备份，如果没有这个字段，或其值为0，则dump程序忽略这个文件系统。

fsck程序根据fsck order字段来决定磁盘的检查顺序，一般来说对于根文件系统这个字段设为1，其他文件系统设为2。如果设为0，则fsck程序忽略这个文件系统。

#### 构建dev目录 ####

本节使用两种方法构建dev目录

1. 静态创建设备文件

	为简单起见，本书先使用最原始的方法处理设备：在/dev目录下静态创建各种节点（即设备文件）。

	从系统启动过程可知，涉及的设备有：/dev/mtdblock*(MTD块设备)、/dev/ttySAC*（串口设备）、/dev/console、/dev/null，只要建立以下设备就可以启动系统。

	    mkdir -p /work/nfs_root/fs_mini/dev
	    cd /work/nfs_root/fs_mini/dev
	    sudo mknod console c 5 1
	    sudo mknod null c 1 3
	    sudo mknod ttySAC0 c 204 64
	    sudo mknod mtdblock0 b 31 0
	    sudo mknod mtdblock1 b 31 1
	    sudo mknod mtdblock2 b 31 2

	注意　在一般系统中，ttySAC0的主设备号为4，但是在S3C2410、S3C2440所用的Linux 2.6.22.6上，它们的串口主设备号为204。

	其他设备文件可以当系统启动后，使用“cat /proc/devices”命令查看内核中注册了哪些设备，然后一一创建相应的设备文件。

	实际上，各个Linux系统中dev目录的内容很相似，本书最终使用的dev目录就是从其他系统中复制过来的。

2. 使用mdev创建设备文件

	mdev是udev的简化版本，它也是通过读取内核信息来创建设备文件。

	mdev的用法请参考busybox-1.7.0/doc/mdev.txt文件。mdev的用途主要有两个：初始化/dev目录、动态更新。动态更新不仅是更新/dev目录，还支持热拔插，即接入、卸下设备时执行某些动作。

	要使用mdev，需要内核支持sysfs文件系统，为了减少对Flash的读写，还要支持tmpfs文件系统。先确保内核已经设置了CONFIG_SYSFS、CONFIG_TMPFS配置项。

	使用mdev的命令如下，请参考它们的注释以了解其作用：

	    mount -t tmpfs mdev /dev	/* 使用内存文件系统，减少对FLASH的读写 */
	    mkdir /dev/pts	/* devpts用来支持外部网络连接(telnet)的虚拟终端 */
	    mount -t devpts devpts /dev/pts
	    mount -t sysfs sysfs /sys	/* mdev通过sysfs文件系统获得设备信息 */
	    echo /bin/mdev > /proc/sys/kernel/hotplug	/* 设置内核，当有设备拔插时调用/bin/mdev程序 */
	    mdev -s	/* 在/dev目录下生成内核支持的所有设备的节点 */

	要在内核启动时，自动运行mdev。这要修改/work/nfs_root/fs_mini中的两个文件：修改etc/fstab来自动挂载文件系统、修改etc/init.d/rcS加入要自动运行的命令。修改后的文件如下：

	etc/fstab

	    # device    mount-point    type    options    dump    fsck order
	    proc         /proc         proc    defaults    0       0
	    tmpfs        /tmp          tmpfs   defaults    0       0
	    sysfs        /sys          sysfs   defaults    0       0
	    tmpfs        /dev          tmpfs   defaults    0       0

	etc/init.d/rcS：加入下面几行

	    mount -a
	    mkdir /dev/pts
	    mount -t devpts devpts /dev/pts
	    echo /sbin/mdev > /proc/sys/kernel/hotplug
	    mdev -s

	需要注意的是，开发板上通过mdev生成的/dev目录中，S3C2410、S3C2440是串口名是s3c2410_serial 0、s3c2410_serial 1、s3c2410_serial 2，不是ttySAC0、ttySAC1、ttySAC2。需要修改etc/inittab文件。

	修改前：

	    ttySAC0::askfirst:-/bin/sh

	修改后：

	    s3c2410_serial0::askfirst:-/bin/sh

	另外，mdev是通过init进程来启动的，在使用mdev构造/dev目录之前，init进程至少要用到设备文件/dev/console、/dev/null，所以要建立这两个设备文件。

	    mkdir -p /work/nfs_root/fs_mini/dev
	    cd /work/nfs_root/fs_mini/dev
	    sudo mknod console c 5 1
	    sudo mknod null c 1 3

#### 构建其他目录 ####

其他目录可以是空目录，比如proc、mnt、tmp、sys、root等，如下创建：

    cd /work/nfs_root/fs_mini
    mkdir proc mnt tmp sys root

现在，/work/nfs_root/fs_mini目录下就是一个非常小的根文件系统。开发板可以将它作为网络根文件系统直接启动。如果要烧入开发板，还要将它制作为一个文件，称为映象文件。

## 制作/使用jffs2文件系统映象文件 ##

### 编译制作jffs2映象文件的工具 ###

mtd-utils-05.07.23.tar.bz2是MTD设备的工具包，编译它生成mkfs.jffs2工具，用它来将一个目录制作成jffs2文件系统映象文件。

这个工具包需要zlib压缩包，先安装zlib。下载源码zlib-1.2.3.tar.gz，执行以下命令进行安装：

    tar xzf zlib-1.2.3.tar.gz
    cd zlib-1.2.3
    ./configure --shared --prefix=/usr
    make
    sudo make install

然后编译mkfs.jffs2：

    tar xjf mtd-utils-07.05.23.tar.bz2
    cd mtd-utils-05.07.03/util
    make
    sudo make install

### 制作/烧写jffs2映象文件 ###

使用如下命令将/work/nfs_root/fs_mini目录制作为fs_mini.jffs2文件：

    cd /work/nfs_root
    mkfs.jffs2 -n -s 2014 -e 128KiB -d fs_mini -o fs_mini.jffs2

上面命令中，“-n”表示不要在每个擦除块上都加上清除标志，“-s 2014”指明一页大小为2014字节，“-e 128KiB”指明一个擦除块大小为128KB，“-d”表示根文件系统目录，“-o”表示输出文件。（要根据NandFlash的具体情况设定参数）

将fs_mini.jffs2放入tftp目录或nfs目录后，在U-Boot控制界面就可以将下载、烧入NAND Flash中，操作命令如下：
    tftp 0x30000000 fs_mini.jffs2 或 nfs 0x30000000 192.168.1.57:/work/nfs_root/fs_mini.jffs2
    nand erase 0x200000 0x800000
    nand write.jffs2 0x30000000 0x200000 $(filesize)
(也可以使用DNW USB下载)

系统启动后，就可以使用“mount -t jffs2 /dev/mtdblock1 /mnt”挂接jffs2文件系统。

也可以修改命令行参数以MTD1分区作为根文件系统，比如在U-Boot控制界面如下设置：

    set bootargs noinitrd console=ttySAC0 root=/dev/mtdblock1 rootfstype=jffs2
    saveenv

## 制作/使用yaffs2文件系统映象文件 ##

### 编译制作yaffs2映象文件的工具 ###

在yaffs源码中有个utils目录（假设这个目录为/work/system/Development/yaffs2/utils），里面是工具mkyaffsimage和mkyaffs2image的源代码。前者用来制作yaffs1映象文件，后者用来制作yaffs2映象文件。

在/work/system/Development/yaffs2/utils目录下执行“make”命令生成mkyaffs2image工具，将它复制到/usr/local/bin目录。

    sudo cp mkyaffs2image /usr/local/bin
    sudo chmod +x /usr/local/bin/mkyaffs2image

### 制作/烧写yaffs映象文件 ###

使用如下命令将/work/nfs_root/fs_mini目录制作为fs_mini.yaffs2文件：

    cd /work/nfs_root
    mkyaffs2image fs_mini fs_mini.yaffs2

将fs_mini.yaffs2放入tftp目录或nfs目录后，在U-Boot控制界面就可以下载、烧入NAND Flash中，操作命令如下：
    tftp 0x30000000 fs_mini.yaffs2 或 nfs 0x30000000 192.168.1.57:/work/nfs_root/fs_mini.yaffs2
    nand erase 0x200000 0x800000
    nand write.yaffs2 0x30000000 0x200000 $(filesize)
(也可以使用DNW USB下载)

