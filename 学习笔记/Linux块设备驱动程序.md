### 1、一些概念 ###

#### （1）硬盘结构（老式磁盘） ####

如图1所示，图中的一圈圈灰色同心圆为一条条磁道，从圆心向外画直线，可以将磁道划分为若干个弧段，每个磁道上一个弧段被称之为一个扇区（图践绿色部分）。扇区是磁盘的最小组成单元，通常是512字节。

![图1 老式磁盘一个盘片的结构](https://i.imgur.com/otYNboD.gif)

图1 老式磁盘一个盘片的结构

图2展示了由一个个盘片组成的磁盘立体结构，一个盘片上下两面都是可读写的，图中蓝色部分叫柱面（cylinder）。

![](https://i.imgur.com/7Fmk9Fi.gif)

图2 老式磁盘的整体结构

简简单介绍了磁盘结构后，下面我们将对磁盘的参数进行讲解。磁盘的常见参数如下：

- 磁头（head）

- 磁道（track）

- 柱面（cylinder）

- 扇区（sector）

- 圆盘（platter）

图2中磁盘是一个3个圆盘6个磁头，7个柱面（每个盘片7个磁道） 的磁盘，图2中每条磁道有12个扇区，所以此磁盘的容量为 `6*7*12*512` 字节。

即：

     存储容量 ＝ 磁头数 × 磁道(柱面)数 × 每道扇区数 × 每扇区字节数

硬盘操作的耗时主要在磁盘转换的时候，如果我们一会读1磁头，一会写2磁头，然后又读1磁头，如果来回这么跳转，会导致效率十分低。

#### （2）Flash ####

Flash存储单元是页（Page）跟硬盘的扇区一样，每一个页的有效容量是512字节的倍数。

Flash以块(Block)为单位进行操作。操作页时也需要读取整个块到内存中。如果读第1块中的第1页，又去写第2块的1页，然后又读第1块的2页，如果不优化效率将很低。

![](https://i.imgur.com/pSsOTR4.jpg)

因此，基于以上硬件的特点及效率问题，内核采用了**电梯调度算法**进行命令的优化。也就是收到一个命令时不会立刻执行，而是放入队列，当存在多个命令时，经过电梯调度算法来进行优化后执行，这样就提升了读写的效率，这个是块设备驱动程序要考虑的问题。

### 2、块设备驱动程序的框架 ###

![](https://i.imgur.com/uKs9sYT.jpg)

应用程序文件读写 -->> 扇区读写，文件系统来做转换之后调用ll_rw_block函数操作块设备。

那么扇区读写函数是？

ll_rw_block：low_level  read write block

ll_rw_block作用：把读写放入队列并优化，之后调用队列的处理函数。即优化后执行。

	分析ll_rw_block
	        for (i = 0; i < nr; i++) {
	            struct buffer_head *bh = bhs[i];
	            submit_bh(rw, bh);
	                struct bio *bio; // 使用bh来构造bio (block input/output)
	                submit_bio(rw, bio);
	                    // 通用的构造请求: 使用bio来构造请求(request)
	                    generic_make_request(bio);
	                        __generic_make_request(bio);
	                            request_queue_t *q = bdev_get_queue(bio->bi_bdev); // 找到队列  
	                            
	                            // 调用队列的"构造请求函数"
	                            ret = q->make_request_fn(q, bio);
	                                    // 默认的函数是__make_request
	                                    __make_request
	                                        // 先尝试合并
	                                        elv_merge(q, &req, bio);
	                                        
	                                        // 如果合并不成，使用bio构造请求
	                                        init_request_from_bio(req, bio);
	                                        
	                                        // 把请求放入队列
	                                        add_request(q, req);
	                                        
	                                        // 执行队列
	                                        __generic_unplug_device(q);
	                                                // 调用队列的"处理函数"
	                                                q->request_fn(q);

主要函数分析：

	int submit_bh(rw, bh);//用buffer_head来构造bio，最后提交bio -->> submit_bio

	generic_make_request(bio);//使用bio来构造请求，把请求放入队列

	/*
	 * 电梯调度算法，来尝试把bio合并到队列里面去。如果合并不成，使用bio构造请求
	 * init_request_from_bio(req, bio)，把请求放入队列add_request(q,req)，执行队列
	 * __generic_unplug_device(q)，调用队列的"处理函数" q->request_fn(q);
	 */
	elv_merge(q, &req, bio);

这里强调：块设备的读写并不会立刻执行，而是先放入队列进行优化。

### 3、写块设备驱动程序的一般过程 ###

常规思路，分配结构体，设置结构体，注册到某个地方。

- 分配gendisk:alloc_disk

- 设置

	分配/设置队列: request_queue_t  // 它提供读写能力

	blk_init_queue                // 设备队列

	设置gendisk其他信息            // 它提供属性: 比如容量

- 注册: add_disk

### 4、实例：内存模拟块设备 ###

- 明确需要做的任务

	1.分配一个gendisk结构体

	2.设置

	2.1分配/设置队列: 提供读写能力

	2.2设置其他属性: 比如容量

	3.注册

- 实现

	ramblock_disk = alloc_disk(16); /* 次设备号个数: 分区个数+1 */

	ramblock_buf= kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);//分配内存模拟

	while ((req = elv_next_request(q)) != NULL)//电梯调度算法

- 所有代码：

参考：

drivers\block\xd.c

drivers\block\z2ram.c

	#include <linux/module.h>
	#include <linux/errno.h>
	#include <linux/interrupt.h>
	#include <linux/mm.h>
	#include <linux/fs.h>
	#include <linux/kernel.h>
	#include <linux/timer.h>
	#include <linux/genhd.h>
	#include <linux/hdreg.h>
	#include <linux/ioport.h>
	#include <linux/init.h>
	#include <linux/wait.h>
	#include <linux/blkdev.h>
	#include <linux/blkpg.h>
	#include <linux/delay.h>
	#include <linux/io.h>
	
	#include <asm/system.h>
	#include <asm/uaccess.h>
	#include <asm/dma.h>
	
	static struct gendisk *ramblock_disk;
	static request_queue_t *ramblock_queue;
	
	static int major;
	
	static DEFINE_SPINLOCK(ramblock_lock);
	
	#define RAMBLOCK_SIZE (1024*1024)
	static unsigned char *ramblock_buf;
	
	static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
	{
		/* 容量=heads*cylinders*sectors*512 */
		geo->heads     = 2;
		geo->cylinders = 32;
		geo->sectors   = RAMBLOCK_SIZE/2/32/512;
		return 0;
	}
	
	
	static struct block_device_operations ramblock_fops = {
		.owner	= THIS_MODULE,
		.getgeo	= ramblock_getgeo,
	};
	
	static void do_ramblock_request(request_queue_t * q)
	{
		static int r_cnt = 0;
		static int w_cnt = 0;
		struct request *req;
		
		//printk("do_ramblock_request %d\n", ++cnt);
	
		while ((req = elv_next_request(q)) != NULL) {
			/* 数据传输三要素: 源,目的,长度 */
			/* 源/目的: */
			unsigned long offset = req->sector * 512;
	
			/* 目的/源: */
			// req->buffer
	
			/* 长度: */		
			unsigned long len = req->current_nr_sectors * 512;
	
			if (rq_data_dir(req) == READ)
			{
				//printk("do_ramblock_request read %d\n", ++r_cnt);
				memcpy(req->buffer, ramblock_buf+offset, len);
			}
			else
			{
				//printk("do_ramblock_request write %d\n", ++w_cnt);
				memcpy(ramblock_buf+offset, req->buffer, len);
			}		
			
			end_request(req, 1);
		}
	}
	
	static int ramblock_init(void)
	{
		/* 1. 分配一个gendisk结构体 */
		ramblock_disk = alloc_disk(16); /* 次设备号个数: 分区个数+1 */
	
		/* 2. 设置 */
		/* 2.1 分配/设置队列: 提供读写能力 */
		ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);
		ramblock_disk->queue = ramblock_queue;
		
		/* 2.2 设置其他属性: 比如容量 */
		major = register_blkdev(0, "ramblock");  /* cat /proc/devices */	
		ramblock_disk->major       = major;
		ramblock_disk->first_minor = 0;
		sprintf(ramblock_disk->disk_name, "ramblock");
		ramblock_disk->fops        = &ramblock_fops;
		set_capacity(ramblock_disk, RAMBLOCK_SIZE / 512);
	
		/* 3. 硬件相关操作 */
		ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);
	
		/* 4. 注册 */
		add_disk(ramblock_disk);
	
		return 0;
	}
	
	static void ramblock_exit(void)
	{
		unregister_blkdev(major, "ramblock");
		del_gendisk(ramblock_disk);
		put_disk(ramblock_disk);
		blk_cleanup_queue(ramblock_queue);
	
		kfree(ramblock_buf);
	}
	
	module_init(ramblock_init);
	module_exit(ramblock_exit);
	
	MODULE_LICENSE("GPL");

- 测试

	在开发板上:
	1. insmod ramblock.ko
	2. 格式化: mkdosfs /dev/ramblock
	3. 挂接: mount /dev/ramblock /tmp/
	4. 读写文件: cd /tmp, 在里面vi文件
	5. 卸载：cd / && umount /tmp/
	6. cat /dev/ramblock > /mnt/ramblock.bin
	7. 在PC上查看ramblock.bin：sudo mount -o loop ramblock.bin /mnt
	8. 建立和操作分区：fdisk /dev/ramblock（然后根据提示进行操作）