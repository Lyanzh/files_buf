#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>

static unsigned int major;
static struct cdev hello_cdev;
static struct cdev hi_cdev;

static struct class *hello_cls;

static int hello_open(struct inode *inode, struct file *file)
{
	printk("open hello\n");
	return 0;
}

static int hi_open(struct inode *inode, struct file *file)
{
	printk("open hi\n");
	return 0;
}

static const struct file_operations hello_fileops = {
	.owner	= THIS_MODULE,
	.open	= hello_open,
};

static const struct file_operations hi_fileops = {
	.owner	= THIS_MODULE,
	.open	= hi_open,
};


static int __init hello_init(void)
{
	dev_t devid;

	if (major) {
		devid = MKDEV(major, 0);
		register_chrdev_region(devid, 2, "hello");
	} else {
		alloc_chrdev_region(&devid, 0, 2, "hello");
		major = MAJOR(devid);
	}

	cdev_init(&hello_cdev, &hello_fileops);
	cdev_add(&hello_cdev, devid, 2);


	devid = MKDEV(major, 2);
	register_chrdev_region(devid, 2, "hi");

	cdev_init(&hi_cdev, &hi_fileops);
	cdev_add(&hi_cdev, devid, 2);

	hello_cls = class_create(THIS_MODULE, "hello");
	class_device_create(hello_cls, NULL, MKDEV(major, 0), NULL, "hello0");
	class_device_create(hello_cls, NULL, MKDEV(major, 1), NULL, "hello1");
	class_device_create(hello_cls, NULL, MKDEV(major, 2), NULL, "hello2");
	class_device_create(hello_cls, NULL, MKDEV(major, 3), NULL, "hello3");

	return 0;
}

static void __exit hello_exit(void)
{
	class_device_destroy(hello_cls, MKDEV(major, 0));
	class_device_destroy(hello_cls, MKDEV(major, 1));
	class_device_destroy(hello_cls, MKDEV(major, 2));
	class_device_destroy(hello_cls, MKDEV(major, 3));
	class_destroy(hello_cls);
	
	cdev_del(&hello_cdev);
	unregister_chrdev_region(MKDEV(major, 0), 2);

	cdev_del(&hi_cdev);
	unregister_chrdev_region(MKDEV(major, 2), 2);
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");

