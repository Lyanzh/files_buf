# Linux字符设备驱动之poll机制 #

## poll机制总结 ##

1. poll > sys_poll > do_sys_poll >poll_initwait，poll_initwait函数注册一下回调函数__pollwait，它就是我们的驱动程序执行poll_wait时，真正被调用的函数；

2. 接下来执行file->f_op->poll，即我们驱动程序里自己实现的poll函数；

	它会调用poll_wait把自己挂入某个队列，这个队列也是我们的驱动自己定义的；

	它还判断一下设备是否就绪；

3. 如果设备未就绪，do_sys_poll里会让进程休眠一定时间

4. 进程被唤醒的条件有2：一是上面说的“一定时间”到了，二是被驱动程序唤醒。驱动程序发现条件就绪时，就把“某个队列”上挂着的进程唤醒，这个队列，就是前面通过poll_wait把本进程挂过去的队列。

5. 如果驱动程序没有去唤醒进程，那么chedule_timeout(__timeou)超时后，会重复2、3动作，直到应用程序的poll调用传入的时间到达。

## 驱动中主要代码 ##

	#include <linux/poll.h>

	static int ev_press = 0;

    static unsigned int fourth_drv_poll(struct file *file, poll_table *wait)
    {
    	unsigned int mask = 0;
      
    	/* 该函数，只是将进程挂在button_waitq队列上，而不是立即休眠 */
    	poll_wait(file, &button_waitq, wait);  
      
    	/* 当没有按键按下时，即不会进入按键中断处理函数，此时ev_press = 0
     	 * 当按键按下时，就会进入按键中断处理函数，此时ev_press被设置为1
     	 */
    	if(ev_press)
    	{
    		mask |= POLLIN | POLLRDNORM;  /* 表示有数据可读 */
    	}  
      
    	/* 如果有按键按下时，mask |= POLLIN | POLLRDNORM,否则mask = 0 */
    	return mask;
    }

    /* File operations struct for character device */
    static const struct file_operations fourth_drv_fops = {
    	.owner  = THIS_MODULE,
    	.open   = fourth_drv_open,
    	.read   = fourth_drv_read,
    	.release= fourth_drv_close,
    	.poll   = fourth_drv_poll,
    };


## 应用测试源码 ##

    int main(int argc ,char *argv[])
    {
    	int fd;
    	unsigned char key_val;
    	struct pollfd fds;
    	int ret;
      
    	fd = open("/dev/buttons",O_RDWR);
    	if (fd < 0)
    	{
    		printf("open error\n");
    	}
    	fds.fd = fd;
    	fds.events = POLLIN;
    	while(1)
    	{
    		/* A value of 0 indicates  that the call timed out and no file descriptors were ready 
     	 	 * poll函数返回0时，表示5s时间到了，而这段时间里，没有事件发生"数据可读"
     	 	 */
    		ret = poll(&fds,1,5000);
    		if(ret == 0)
    		{
    			printf("time out\n");
    		}
    		else/* 如果没有超时，则读出按键值 */
    		{
    			read(fd,&key_val,1);
    			printf("key_val = 0x%x\n",key_val);
    		}
    	}  
    	return 0;
    }

当按键没有被按下时，5秒之后，会显示出time out，表示时间已到，在这5秒时间里，没有按键被按下，即没有数据可读，当按键按下时，立即打印出按下的按键；同时，fourth_test进程，也几乎不占用CPU的利用率。