1. printk及控制台的日志级别

	函数printk的使用方法和printf相似，用于内核打印消息。printk根据日志级别（loglevel）对消息进行分类。

	日志级别用宏定义，日志级别宏展开为一个字符串，在编译时由预处理器将它和消息文本拼接成一个字符串，因此printk 函数中日志级别宏和格式字符串间不能有逗号。

	下面是两个printk的例子，一个用于打印调试信息，另一个用于打印临界条件信息。


		printk(KERN_DEBUG "Here I am: %s:%i/n", __FILE__, __LINE__);
		
		printk(KERN_CRIT "I'm trashed; giving up on %p/n", ptr);

	printk的日志级别定义如下（在linux26/includelinux/kernel.h中）：

		#defineKERN_EMERG"<0>"/*紧急事件消息，系统崩溃之前提示，表示系统不可用*/
		
		#defineKERN_ALERT"<1>"/*报告消息，表示必须立即采取措施*/
		
		#defineKERN_CRIT"<2>"/*临界条件，通常涉及严重的硬件或软件操作失败*/
		
		#defineKERN_ERR"<3>"/*错误条件，驱动程序常用KERN_ERR来报告硬件的错误*/
		
		#defineKERN_WARNING"<4>"/*警告条件，对可能出现问题的情况进行警告*/
		
		#defineKERN_NOTICE"<5>"/*正常但又重要的条件，用于提醒。常用于与安全相关的消息*/
		
		#defineKERN_INFO"<6>"/*提示信息，如驱动程序启动时，打印硬件信息*/
		
		#defineKERN_DEBUG"<7>"/*调试级别的消息*/
		
		
		extern int console_printk[];
		
		
		#define console_loglevel 　(console_printk[0])
		
		#define default_message_loglevel　 (console_printk[1])
		
		#define minimum_console_loglevel　 (console_printk[2])
		
		#define default_console_loglevel　 (console_printk[3])

	日志级别的范围是0～7，没有指定日志级别的printk语句默认采用的级别是 DEFAULT_ MESSAGE_LOGLEVEL，其定义列出如下（在linux26/kernel/printk.c中）：
	
		/*没有定义日志级别的printk使用下面的默认级别*/
		#define DEFAULT_MESSAGE_LOGLEVEL 4 /* KERN_WARNING 警告条件*/

	内核可把消息打印到当前控制台上，可以指定控制台为字符模式的终端或打印机等。默认情况下，“控制台”就是当前的虚拟终端。
	
	为了更好地控制不同级别的信息显示在控制台上，内核设置了控制台的日志级别console_loglevel。printk日志级别的作用是打印一定级别的消息，与之类似，控制台只显示一定级别的消息。
	
	当日志级别小于console_loglevel时，消息才能显示出来。控制台相应的日志级别定义如下：

		/* 显示比这个级别更重发的消息*/
		
		#define MINIMUM_CONSOLE_LOGLEVEL　 1　 /*可以使用的最小日志级别*/
		
		#define DEFAULT_CONSOLE_LOGLEVEL 　7 /*比KERN_DEBUG 更重要的消息都被打印*/
		
		
		int console_printk[4] = {
		
			DEFAULT_CONSOLE_LOGLEVEL,/*控制台日志级别，优先级高于该值的消息将在控制台显示*/
		
			/*默认消息日志级别，printk没定义优先级时，打印这个优先级以上的消息*/
			DEFAULT_MESSAGE_LOGLEVEL,
		
			/*最小控制台日志级别，控制台日志级别可被设置的最小值（最高优先级）*/
			MINIMUM_CONSOLE_LOGLEVEL,
		
			DEFAULT_CONSOLE_LOGLEVEL,/* 默认的控制台日志级别*/
		};

	如果系统运行了klogd和syslogd，则无论console_loglevel为何值，内核消息都将追加到/var/log/messages中。如果klogd没有运行，消息不会传递到用户空间，只能查看/proc/kmsg。

	变量console_loglevel的初始值是DEFAULT_CONSOLE_LOGLEVEL，可以通过sys_syslog系统调用进行修 改。调用klogd时可以指定-c开关选项来修改这个变量。如果要修改它的当前值，必须先杀掉klogd，再加-c选项重新启动它。

	通过读写/proc/sys/kernel/printk文件可读取和修改控制台的日志级别。查看这个文件的方法如下：

		#cat /proc/sys/kernel/printk
			6   4  1   7

	上面显示的4个数据分别对应控制台日志级别、默认的消息日志级别、最低的控制台日志级别和默认的控制台日志级别。

	可用下面的命令设置当前日志级别：

		# echo 8 > /proc/sys/kernel/printk

