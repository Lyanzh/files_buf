图 5.1 所示为 Linux 中虚拟文件系统、磁盘文件（存放于 Ramdisk、 Flash、 ROM、 SD 卡、 U盘等文件系统中的文件也属于此列）及一般的设备文件与设备驱动程序之间的关系。

![](https://i.imgur.com/RS1JRHM.jpg)

**应用程序和 VFS 之间的接口是系统调用，而 VFS 与磁盘文件系统以及普通设备之间的接口是 file_operations 结构体成员函数**，这个结构体包含对文件进行打开、关闭、读写、控制的一系列成员函数。

由于字符设备的上层没有磁盘文件系统，所以字符设备的 file_operations 成员函数就直接由设备驱动提供了，file_operations 正是字符设备驱动的核心。

而对于块存储设备而言， ext2、 fat、 jffs2 等文件系统中会实现针对 VFS 的 file_operations 成员函数，**设备驱动层将看不到 file_operations 的存在**。磁盘文件系统和设备驱动会将对磁盘上文件的访问最终转换成对磁盘上柱面和扇区的访问。

在设备驱动程序的设计中，一般而言，会关心 file 和 inode 这两个结构体。

1. file 结构体  
	file 结构体代表一个打开的文件（设备对应于设备文件），系统中每个打开的文件在内核空间都有一个关联的 struct file。它由内核在打开文件时创建，并传递给在文件上进行操作的任何函数。  

	在文件的所有实例都关闭后，内核释放这个数据结构。在内核和驱动源代码中， struct file 的指针通常被命名为 file 或 filp（即 file pointer）。  

	代码清单 5.3 给出了文件结构体的定义。

	代码清单 5.3 文件结构体

		truct file
		{
			nion {
				truct list_head fu_list;
				truct rcu_head fu_rcuhead;
			} f_u;
			truct dentry *f_dentry; /*与文件关联的目录入口(dentry)结构*/
			truct vfsmount *f_vfsmnt;
			truct file_operations *f_op; /* 和文件关联的操作*/
			atomic_t f_count;
			unsigned int f_flags;/*文件标志，如 O_RDONLY、 O_NONBLOCK、 O_SYNC*/
			mode_t f_mode; /*文件读/写模式， FMODE_READ 和 FMODE_WRITE*/
			loff_t f_pos; /* 当前读写位置*/
			struct fown_struct f_owner;
			unsigned int f_uid, f_gid;
			struct file_ra_state f_ra;
			
			unsigned long f_version;
			void *f_security;
			
			/* tty 驱动需要，其他的也许需要 */
			void *private_data; /*文件私有数据*/
			...
			struct address_space *f_mapping;
		};

	文件读/写模式 mode、标志 f_flags 都是设备驱动关心的内容，而私有数据指针 private_data在设备驱动中被广泛应用，大多被指向设备驱动自定义用于描述设备的结构体。  

	驱动程序中经常会使用如下类似的代码来检测用户打开文件的读写方式：

		if (file->f_mode & FMODE_WRITE) {/* 用户要求可写 */
			...
		}
		if (file->f_mode & FMODE_READ) {/* 用户要求可读 */
			...
		}

	下面的代码可用于判断以阻塞还是非阻塞方式打开设备文件：

		if (file->f_flags & O_NONBLOCK) /* 非阻塞 */
			pr_debug("open: non-blocking\n");
		else /* 阻塞 */
			pr_debug("open: blocking\n");

2. inode 结构体

	VFS inode 包含文件访问权限、属主、组、大小、生成时间、访问时间、最后修改时间等信息。它是 Linux 管理文件系统的最基本单位，也是文件系统连接任何子目录、文件的桥梁， inode结构体的定义如代码清单 5.4 所示。

	代码清单 5.4 inode 结构体

		struct inode {
		...
			umode_t i_mode; /* inode 的权限 */
			uid_t i_uid; /* inode 拥有者的 id */
			gid_t i_gid; /* inode 所属的群组 id */
			dev_t i_rdev; /* 若是设备文件，此字段将记录设备的设备号 */
			loff_t i_size; /* inode 所代表的文件大小 */
			
			struct timespec i_atime; /* inode 最近一次的存取时间 */
			struct timespec i_mtime; /* inode 最近一次的修改时间 */
			struct timespec i_ctime; /* inode 的产生时间 */
			
			unsigned long i_blksize; /* inode 在做 I/O 时的区块大小 */
			unsigned long i_blocks; /* inode 所使用的 block 数，一个 block 为 512 byte*/
			
			struct block_device *i_bdev;
			/*若是块设备，为其对应的 block_device 结构体指针*/
			struct cdev *i_cdev; /*若是字符设备，为其对应的 cdev 结构体指针*/
			...
		};

	对于表示设备文件的 inode 结构， i_rdev 字段包含设备编号。 Linux 2.6 设备编号分为主设备编号和次设备编号，前者为 dev_t 的高 12 位，后者为 dev_t 的低 20 位。

	下列操作用于从一个 inode中获得主设备号和次设备号：

		unsigned int iminor(struct inode *inode);
		unsigned int imajor(struct inode *inode);

	查看/proc/devices 文件可以获知系统中注册的设备，第 1 列为主设备号，第 2 列为设备名，如：

		Character devices:
		1 mem
		2 pty
		3 ttyp
		4 /dev/vc/0
		4 tty
		5 /dev/tty
		5 /dev/console
		5 /dev/ptmx
		7 vcs
		10 misc
		13 input
		21 sg
		29 fb
		128 ptm
		136 pts
		171 ieee1394
		180 usb
		189 usb_device
		Block devices:
		1 ramdisk
		2 fd
		8 sd
		9 md
		22 ide1
		...

	查看/dev 目录可以获知系统中包含的设备文件，日期的前两列给出了对应设备的主设备号和次设备号：

		crw-rw---- 1 root uucp 4, 64 Jan 30 2003 /dev/ttyS0
		brw-rw---- 1 root disk 8, 0 Jan 30 2003 /dev/sda

	主设备号是与驱动对应的概念，同一类设备一般使用相同的主设备号，不同类的设备一般使用不同的主设备号（但是也不排除在同一主设备号下包含有一定差异的设备）。因为同一驱动可支持多个同类设备，因此用次设备号来描述使用该驱动的设备的序号，序号一般从 0 开始。

	内核 Documents 目录下的 devices.txt 文件描述了 Linux 设备号的分配情况。