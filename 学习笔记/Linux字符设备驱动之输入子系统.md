# Linux字符设备驱动之输入子系统 #

平常我们的按键，触摸屏，鼠标等输入型设备都可以利用input接口来简化驱动程序并实现设备驱动。

## 输入子系统原理 ##

linux输入子系统的体系结构可以分为三个层面，分别为：驱动层、输入核心层、事件处理层，三个有点类似PHP的MVC模式，意思就是每个层次只是负责单独的一个功能，无需参与其他的功能，有点类似函数的封装，三个层面具体的功能如下：

- 驱动层：主要实现对硬件设备的读写访问，中断设置，将底层的硬件输入转化为统一的事件类型，向输入核心（input core）汇报事件，简单来说，驱动层就是负责汇报事情。

- 输入核心层：为驱动层提供输入设备的注册和操作接口。设备驱动层只要关心如何驱动硬件并获得硬件数据（例如按下的按键数据），然后调用核心层提供的接口，核心层会自动把数据提交给事件处理层。

	- 用input_register_device函数对设备进行注册；

	- 通知事件处理层对事件进行处理；

	- 在/proc下产生相应的设备信息。

- 事件处理层：主要作用就是与用户空间进行交互。包含提供驱动程序的fops接口，在/dev下生成相应的设备文件节点nod等功能。

/dev/input目录下显示的是已经注册在内核中的设备编程接口，用户通过open这些设备文件来打开不同的输入设备进行硬件操作。

事件处理层为不同硬件类型提供了用户访问及处理接口。例如当我们打开设备/dev/input/mice时，会调用到事件处理层的Mouse Handler来处理输入事件，这也使得设备驱动层无需关心设备文件的操作，因为Mouse Handler已经有了对应事件处理的方法。

输入子系统由内核代码drivers/input/input.c构成，它的存在屏蔽了用户到设备驱动的交互细节，为设备驱动层和事件处理层提供了相互通信的统一界面。

下图简单描述了linux输入子系统的事件处理机制：

![](http://i.imgur.com/27M89ei.png)

归纳一下：

一个事件，如鼠标移动，键盘按下事件，首先通过驱动层Driver --> 输入核心层 InputCore-->事件处理层Event handler-->用户空间Userspace的顺序来完成事件的响应。

## 驱动实现 ##

### 1.设备描述 ###

input设备用input_dev结构体来描述。

在input子系统实现设备驱动程序中，驱动的核心工作是向系统报告按键，触摸屏，鼠标等事件，无须关心文件操作接口，因为这些接口是有事件处理层Event handler来实现的。

### 2.事件支持 ###

首先一个设备驱动，我们应该通过set_bit()函数来告诉输入子系统它支持哪些事件，哪些按键，例如：

	set_bit(EV_KEY, button_dev.evbit);//告诉输入子系统支持按键的事件

struct input_dev其中两个成员：

      evbit : 事件类型
      keybit: 按键类型


事件类型：

	EV_RST            //reset
	EV_KEY            //按键
	EV_REL            //相对坐标
	EV_ABS            //绝对坐标
	EV_MSC            //其他
	EV_LEC            //LED
	EV_SND            //声音
	EV_REP            //repeat
	EV_FF             //力反馈

但事件类型为EV_KEY时，还需指明按键类型：

	BTN_LEFT:鼠标左键
	BTN_0：数字0键
	BTN_RIGHT:鼠标右键
	BTN_1：数字1键
	BTN_MIDDLE:鼠标中键

### 3.报告事件 ###

当事件真的发生的时间，我们的驱动层应该向输入核心层Input Core来报告EV_KEY,EV_REL,EV_ABS等事件，报告函数分别为：

	void input_report_key(struct input_dev *dev,unsigned int code, int value)

	void input_report_rel(struct input_dev *dev,unsigned int code, int value)

	void input_report_abs(struct input_dev *dev,unsigned int code, int value)

code: 事件的代码：如果事件类型是EV_KEY, 则该代码则为设备的键盘代码，例如键盘上按键代码值为0~127 ，鼠标键代码值为0x110 ~ 0x116 具体请参考include/linux/input.h文件

value：事件的值。如果事件类型是EV_KEY, 按键按下时值为1，松开即为0

### 4.报告结束 ###

	input_sync()//用于告诉输入核心层 input core 此次报告已经结束

例如，在触摸屏设备驱动中，一次点击的整个报告事件过程如下：

	input_report_abs(input_dev, ABS_X, x);	//报告X坐标

	input_report_abs(input_dev, ABS_Y, y);	//报告Y坐标

	input_report_abs(input_dev, ABS_PRESSURE, 1);//报告事件类型为按下，且value值为1

	input_sync(input_dev);	//报告完毕，同步事件

### 驱动程序的主要部分 ###

	//在按键中断中报告事件
	static void buton_interrupt(int irq, void *dummy, struct pt_regs *fp)
	{
		//注意，此处所有的按键都要报告，无论是0号按键还是1号按键
		input_report_key(&button_dev, BTN_0, inb(BUTTON_PORT0));
		input_report_key(&button_dev, BTN_1, inb(BUTTON_PORT1));

		input_sync(&button_dev);//报告完毕，同步事件

		//此时，输入核心层和事件处理层就会将收集的事件类型形成相应的数据，放到file operation 中和 buffer中，以用用户空间读取
	}

	//驱动初始化函数
	static int __init  button_init(void)
	{
		//申请中断,因为按键事件报告是在中断中执行
		if(request_irq(BUTTON_IRQ ,button_interrupt, 0, “button”, NULL))
			return –EBUSY;

		set_bit(EV_KEY, button_dev.evbit);	//告诉输入子系统支持EV_key事件
		set_bit(BTN_0, button_dev.keybit);	//告诉输入子系统支持0号键
		set_bit(BTN_1, button_dev.keybit);	//告诉输入子系统支持1号键

		input_register_device(&button_dev);	//注册input设备
	}

### 测试方法 ###

方法一：

	#hexdump /dev/event1  (open(/dev/event1), read(), )
	          秒(4B)   微秒(4B)   类  code value(4B)
	0000000 0bb2 0000 0e48 000c 0001 0026 0001 0000
	0000010 0bb2 0000 0e54 000c 0000 0000 0000 0000
	0000020 0bb2 0000 5815 000e 0001 0026 0000 0000
	0000030 0bb2 0000 581f 000e 0000 0000 0000 0000

方法二：

如果没有启动QT：

    #cat /dev/tty1

依次按下s2,s3,s4就可以得到 ls

或者：把标准输入文件改为tty1，0代表标准输入，1代表标准输出，2代表标准错误

	#exec 0</dev/tty1

然后可以使用按键来输入

### 应用程序实现 ###

当相应的时间响应，用户空间读取事件是，所读取到的是 input_event 结构的信息，不再是一个单纯的数字，在input_event 结构中，已经包含 按键类型type, 按键键值code等信息。

用户需要对 input_event 进行相应的解析，获得相应的信息。

	struct input_event ev_key;	//声明结构体

	button_fd = open(“/dev/event0”, O_RDWR);

	while(1)
	{
		count = read(button_fd, &ev_key, sizeof(struct input_event));

		for(i = 0; i < (int)count/sizeof(struct input_event); i++)
		{
			if(EV_KEY == ev_key.type)	//确定类型是否相同
				printf(“type:%d, code:%d, value:%d \n”,ev_key.type,ev_key.code,ev_key.value);

			if(EV_SYN == ev_key.type)
				printf(“syn event \n”);
		}
	}

	close(button_fd);

