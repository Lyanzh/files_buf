#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

static struct i2c_driver at24cxx_driver;

static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END };

static struct i2c_client_address_data addr_data = {
	.normal_i2c	= normal_addr,
	.probe		= ignore,
	.ignore		= ignore,
};

static unsigned int major;
static struct class * cls;

static ssize_t at24cxx_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

static ssize_t at24cxx_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

static struct file_operations at24cxx_fops =
{
	.owner = THIS_MODULE,
	.read  = at24cxx_read,
	.write = at24cxx_write,
}

/* This function is called by i2c_probe */
static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *at24cxx_client;
	int err = 0;

	/* There are three ways we can read the EEPROM data:
	   (1) I2C block reads (faster, but unsupported by most adapters)
	   (2) Consecutive byte reads (100% overhead)
	   (3) Regular byte data reads (200% overhead)
	   The third method is not implemented by this driver because all
	   known adapters support at least the second. */
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_READ_BYTE_DATA
					    | I2C_FUNC_SMBUS_BYTE))
		goto exit;

	if (!(at24cxx_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	//i2c_set_clientdata(new_client, data);
	at24cxx_client->addr = address;
	at24cxx_client->adapter = adapter;
	at24cxx_client->driver = &at24cxx_driver;
	at24cxx_client->flags = 0;

	/* Fill in the remaining client fields */
	strlcpy(at24cxx_client->name, "at24cxx", I2C_NAME_SIZE);

	/* Tell the I2C layer a new client has arrived */
	if ((err = i2c_attach_client(at24cxx_client)))
		goto exit_detach;

	major = register_chrdev(0, "at24cxx", &at24cxx_fops);

	cls = class_create(THIS_MODULE, "at24cxx");

	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "at24cxx");

	return 0;

exit_detach:
	i2c_detach_client(at24cxx_client);
exit_kfree:
	kfree(at24cxx_client);
exit:
	return err;
}

static int at24cxx_attach_adapter(struct i2c_adapter *adapter)
{
	printk("at24cxx_attach_adapter");
	return i2c_probe(adapter, &addr_data, at24cxx_detect);
}

static int at24cxx_detach_client(struct i2c_client *client)
{
	int err;
	
	printk("at24cxx_detach_client");

	err = i2c_detach_client(client);
	if (err)
		return err;

	kfree(i2c_get_clientdata(client));

	return 0;
}


/* This is the driver that will be inserted */
static struct i2c_driver at24cxx_driver = {
	.driver = {
		.name	= "at24cxx",
	},
	.id		= I2C_DRIVERID_EEPROM,
	.attach_adapter	= at24cxx_attach_adapter,
	.detach_client	= at24cxx_detach_client,
};


static int __init at24cxx_init(void)
{
	return i2c_add_driver(&at24cxx_driver);
}

static void __exit at24cxx_exit(void)
{
	return i2c_del_driver(&at24cxx_driver);
}

module_init(at24cxx_init);
module_exit(at24cxx_exit);
MODULE_LICENSE("GPL");

