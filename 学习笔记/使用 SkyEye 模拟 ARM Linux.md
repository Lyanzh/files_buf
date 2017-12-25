使用 SkyEye 模拟 ARM Linux

一、 安装 SkyEye

1. 方法一：编译源码安装

	没有成功，所需要的依赖太多太杂，理不清，错误不断，故放弃此方法。

2. Ubuntu下直接 apt-get

	Ubuntu12.04 里：

		apt-get install skyeye

二、  testsuite 测试文件

获取skyeye源码里的testsuite，切换到arm_hello文件夹下，执行：

	skyeye -e arm_hello

如果不断输出helloworld，说明SkyEye安装成功了，截图如下：

![](https://i.imgur.com/FgRZVN6.jpg)

三、 模拟 s3c2410

1、首先在 testsuite 下建立 myhello 目录。

2、添加 myhello.c 文件，输入如下内容：

	#define INTERVAL 100000 
	void myhello(void) 
	{
		long * addr = (long *)0x50000020;
		int timeout = 0;
	
		while(1) {
			timeout = 0;
			while (++timeout <= INTERVAL);
	
			*addr = 'a';
		}
	}

地址 0x50000020 就是 UART 的通道 0（UTXH0）的发送缓冲，把数据写入这个地址就会自动发送出去，当然在模拟器中，发送的目标地址就是我们的屏幕啦。

3、准备启动代码。

myhello.c 写好了之后，我们还要准备一段 s3c2410 的启动代码，这段代码在 s3c2410 一上电之后就开始执行，在这段启动代码中，回跳转到我们写的 myhello.c 函数。start.S 文件：

	.text 
		.align 4 
		.global _start 
	
	_start: 
		ldr sp, =1024*4 
		bl  myhello 
	
	halt: 
		b halt

上面这段很简单，就是声明了一个 _start 标记，这个标记在下面会用到，作为程序的入口地址。汇编和 C 链接的唯一必须的一步就是设置堆栈，这里我们把 sp 指向 4k 顶部，然后跳转到我们的 c 函数myhello 。

4、编写链接脚本。

链接的顺序就是先 start.S 后 myhello.c，myhello.lds 文件：

	OUTPUT_ARCH(arm) 
	ENTRY(_start) 
	SECTIONS 
	{ 
	  . = 0x00000000; 
	  .text :   
	  { 
	    start.o 
	    myhello.o 
	    *(.rodata) 
	  } 
	
	  . = ALIGN(8192); 
	
	  .data : {*(.data)} 
	
	  .bss : {*(.bss)} 
	}

表示输出 arm 格式，第二句表示入口点是 _start 标记，就是第3步的那个 _start 标记，然后在 0x00000000 处先插入 start.o，然后插入 myhello.o 。

5、编写 Makefile 文件。

	CC=arm-linux-gcc
	LD=arm-linux-ld
	CFLAGS= -c -g -march=armv4t -mtune=arm920t
	LDFLAGS= -N -p -X -Tmyhello.lds
	
	myhello: start.o myhello.o
		$(LD) $(LDFLAGS)  start.o myhello.o -o myhello
		arm-linux-objdump -xS myhello > myhello.s
		arm-linux-readelf -a myhello > myhello.r
		arm-linux-nm myhello > myhello.n 
	
	start.o: start.S
		$(CC) $(CFLAGS) start.S
	
	myhello.o: myhello.c
		$(CC) $(CFLAGS) myhello.c
	
	clean:
		rm -rf *.o myhello *.r *.n *.s

6、最后我们还需要一个 skyeye 配置文件。

	#skyeye config file 
	arch:arm 
	cpu: arm920t 
	mach: s3c2410x 
	
	# boot  
	mem_bank: map=M, type=RW, addr=0x00000000, size=0x04000000, boot=yes 
	
	# physical memory  
	mem_bank: map=M, type=RW, addr=0x30000000, size=0x02000000 
	
	# all peripherals I/O mapping area  
	mem_bank: map=I, type=RW, addr=0x48000000, size=0x20000000 
	
	uart:mod=term 
	#log: logon=0, logfile=./sk1.log, start=0, end=200000

7、编译。

	$ make

8、测试。

	$ skyeye -e myhello

我们会发现连续输出了字符“a”，完成！