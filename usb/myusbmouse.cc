#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

/*
 * Version Information
 */
#define DRIVER_VERSION "v1.6"
#define DRIVER_AUTHOR "Vojtech Pavlik <vojtech@ucw.cz>"
#define DRIVER_DESC "USB HID Boot Protocol mouse driver"
#define DRIVER_LICENSE "GPL"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

static struct input_dev *usb_input_dev;
static char *usb_buf;
static dma_addr_t usb_buf_phys;
static int len;
static struct urb *myusb_urb;

static void myusbmouse_irq(struct urb *urb)
{
	int i;
	static int cnt = 0;
	printk("data cnt %d: ", ++cnt);
	for (i = 0; i < len; i++)
	{
		printk("%02x ", usb_buf[i]);
	}
	printk("\n");

	/* 重新提交urb */
	usb_submit_urb(myusb_urb, GFP_KERNEL);
}

static int myusbmouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	int pipe;
	
	interface = intf->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1)//端点数量
		return -ENODEV;

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	usb_input_dev = input_allocate_device();

	set_bit(EV_KEY, usb_input_dev->evbit);
	set_bit(EV_REP, usb_input_dev->evbit);

	set_bit(KEY_L, usb_input_dev->keybit);
	set_bit(KEY_S, usb_input_dev->keybit);
	set_bit(KEY_ENTER, usb_input_dev->keybit);

	input_register_device(usb_input_dev);

	pipe = usb_rcvintpipe(usb_dev, endpoint->bEndpointAddress);

	len = endpoint->wMaxPacketSize;

	usb_buf = usb_buffer_alloc(usb_dev, len, GFP_ATOMIC, &usb_buf_phys);

	myusb_urb = usb_alloc_urb(0, GFP_KERNEL);

	usb_fill_int_urb(myusb_urb, usb_dev, pipe, usb_buf, len, myusbmouse_irq, NULL, endpoint->bInterval);

	myusb_urb->transfer_dma = usb_buf_phys;
	myusb_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	usb_submit_urb(myusb_urb, GFP_KERNEL);

	return 0;
}

void myusbmouse_disconnect(struct usb_interface *intf)
{
	struct usb_device *usb_dev = interface_to_usbdev(intf);

	usb_kill_urb(myusb_urb);
	usb_free_urb(myusb_urb);

	usb_buffer_free(usb_dev, len, usb_buf, usb_buf_phys);
	input_unregister_device(usb_input_dev);
	input_free_device(usb_input_dev);
}

static struct usb_device_id myusbmouse_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, usb_mouse_id_table);
static struct usb_driver myusbmouse_driver = {
	.name = "my_usb_mouse",
	.probe = myusbmouse_probe,
	.disconnect = myusbmouse_disconnect,
	.id_table = myusbmouse_id_table,
};

static int myusbmouse_init(void)
{
	int retval = usb_register(&myusbmouse_driver);
	if (retval == 0)
		info(DRIVER_VERSION ":" DRIVER_DESC);
	return retval;
}

static void myusbmouse_exit(void)
{
	usb_deregister(&myusbmouse_driver);
}

module_init(myusbmouse_init);
module_exit(myusbmouse_exit);

