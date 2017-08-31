## USB driver overview ##

![](http://i.imgur.com/DeZFVfB.jpg)

The USB host controller is in charge of asking every USB device if it has any data to send.

The Linux kernel supports two main types of USB drivers: drivers on a host system
and drivers on a device.

USB drivers live between the different kernel subsytems (block, net, char, etc.) and the USB hardware controllers.

The USB core provides an interface for USB drivers to use to access and control the USB hardware.

## USB Device Basics ##

![](http://i.imgur.com/xz9UcYX.jpg)

Figure 13-2 shows how USB devices consist of configurations, interfaces, and endpoints and how USB drivers bind to USB interfaces, not the entire USB device.

### USB endpoint ###

the chanel of USB communication,can carry data in only one direction.

types of a USB endpoint(base on describing how the data is transmitted):

1. CONTROL(endpoint 0)

	- configuring the device

	- retrieving information about the device

	- sending commands to the device

	- retrieving status reports about the device

2. INTERRUPT

	common on USB keyboards and mice

	- transfer small amounts of data

	- send data to USB devices to control the device

	- always have enough reserved bandwidth to make it through

3. BULK

	common on printers, storage, and network devices

	- transfer large amounts of data(any data that must get through with no data loss)
	
	- no always in a specific amount of time

4. ISOCHRONOUS

	common on real-time data collections, such as audio and video devices

	- transfer large amounts of data, but the data is not always guaranteed to make it through

### USB endpoints described in the kernel ###

USB endpoints are described in the kernel with the structure struct `usb_host_endpoint.`

This structure contains the real endpoint information in another structure called struct `usb_endpoint_descriptor`.

The fields of structure `usb_endpoint_descriptor` that drivers care about:

- bEndpointAddress

	the USB address of this specific endpoint

- bmAttributes

	the type of endpoint

- wMaxPacketSize

	the maximum size in bytes that this endpoint can handle at once

- bInterval

	the time between interrupt requests for the endpoint(only if this endpoint is of type interrupt)

### USB interfaces ###

USB endpoints are bundled up into interfaces.

a USB interface represents basic functionality, each USB driver controls an interface.

USB interfaces handle only one type of a USB logical connection, such as a mouse, a keyboard, or a audio stream.

Some USB devices have multiple interfaces, such as a USB speaker that might consist of two interfaces: a USB keyboard for the buttons and a USB audio stream.

described in the kernel with the struct `usb_interface` structure.

This structure is what the USB core passes to USB drivers and is what the USB driver then is in charge of controlling.

important fields:

- struct usb_host_interface *altsetting

	containing all of the alternate settings that may be selected for this interface.

	consists of a set of endpoint configurations as defined by the struct usb_host_endpoint structure.

- unsigned num_altsetting

	The number of alternate settings pointed to by the altsetting pointer.

- struct usb_host_interface *cur_altsetting

	A pointer into the array altsetting, denotingthe currently active settingfor this interface.

- int minor

	this variable contains the minor number assigned by the USB core to the interface. (valid only after a successful call to usb_register_dev, the USB driver can bound to this interface uses the USB major number).

### Configurations ###

A USB device can have multiple configurations and might switch between them in order to change the state of the device.

A single configuration can be enabled only at one point in time.

Linux describes USB configurations with the structure struct usb_host_config and
entire USB devices with the structure struct usb_device.

### USB and Sysfs ###

Both the physical USB device (as represented by a struct usb_device) and the individual USB interfaces (as represented by a struct usb_interface) are shown in sysfs as individual devices.

### USB Urbs ###

A urb(USB request block) is used to send or receive data to or from a specific USB endpoint on a specific USB device in an asynchronous manner.

described with the struct urb structure.

Every endpoint in a device can handle a queue of urbs.

The typical lifecycle of a urb is as follows:

- Created by a USB device driver.

- Assigned to a specific endpoint of a specific USB device.

- Submitted to the USB core, by the USB device driver.

- Submitted to the specific USB host controller driver for the specified device by the USB core.

- Processed by the USB host controller driver that makes a USB transfer to the
device.

- When the urb is completed, the USB host controller driver notifies the USB
device driver.

## Writing a USB Driver ##

### What Devices Does the Driver Support? ###

The struct usb_device_id structure provides a list of different types of USB devices that this driver supports. This list is used by the USB core to decide which driver to give a device to, and by the hotplug scripts to decide which driver to automatically load when a specific device is plugged into the system.

- USB_DEVICE(vendor, product)

	Creates a struct usb_device_id that can be used to match only the specified vendor and product ID values. This is very commonly used for USB devices that need a specific driver.

- USB_DEVICE_VER(vendor, product, lo, hi)

	Creates a struct usb_device_id that can be used to match only the specified vendor and product ID values within a version range.

- USB_DEVICE_INFO(class, subclass, protocol)

	Creates a struct usb_device_id that can be used to match a specific class of USB devices.

- USB_INTERFACE_INFO(class, subclass, protocol)

	Creates a struct usb_device_id that can be used to match a specific class of USB interfaces.

So, for a simple USB device driver that controls only a single USB device from a single vendor, the struct usb_device_id table would be defined as:

	/* table of devices that work with this driver */
	static struct usb_device_id skel_table [ ] = {
		{ USB_DEVICE(USB_SKEL_VENDOR_ID, USB_SKEL_PRODUCT_ID) },
		{ } /* Terminating entry */
	};
	MODULE_DEVICE_TABLE (usb, skel_table);

### Registering a USB Driver ###

The main structure that all USB drivers must create is a struct usb_driver(must be filled out by the USB driver and consists of a number of function callbacks and variables that describe the USB driver to the USB core code).

To create a value struct usb_driver structure, only five fields need to be initialized:

	static struct usb_driver skel_driver = {
		.owner = THIS_MODULE,//module owner of this driver
		.name = "skeleton",//name of the driver
		.id_table = skel_table,//contains a list of all of the different kinds of USB devices this driver can accept
		.probe = skel_probe,//probe function
		.disconnect = skel_disconnect,//disconnect function
	};

To register the struct usb_driver with the USB core, a call to usb_register_driver is made with a pointer to the struct usb_driver.
	
	static int __init usb_skel_init(void)
	{
		int result;
		/* register this driver with the USB subsystem */
		result = usb_register(&skel_driver);
		if (result)
			err("usb_register failed. Error number %d", result);
		return result;
	}

When the USB driver is to be unloaded, the struct usb_driver needs to be unregistered from the kernel. This is done with a call to usb_deregister_driver.

	static void __exit usb_skel_exit(void)
	{
		/* deregister this driver with the USB subsystem */
		usb_deregister(&skel_driver);
	}

### probe and disconnect in Detail ###

The probe function is called when a device is installed that the USB core thinks this driver should handle; the probe function should perform checks on the information passed to it about the device and decide whether the driver is really appropriate for that device.

The disconnect function is called when the driver should no longer control the device for some reason and can do clean-up.

Here is some example code that detects both IN and OUT endpoints of BULK type and saves some information about them in a local device structure:

	/* set up the endpoint information */
	/* use only the first bulk-in and bulk-out endpoints */
	iface_desc = interface->cur_altsetting;
	
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;//assigns a local pointer to the endpoint structure 
		
		if (!dev->bulk_in_endpointAddr &&
			(endpoint->bEndpointAddress & USB_DIR_IN) &&
			((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
			= = USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk in endpoint */
			buffer_size = endpoint->wMaxPacketSize;
			dev->bulk_in_size = buffer_size;
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			
			if (!dev->bulk_in_buffer) {
				err("Could not allocate bulk_in_buffer");
				goto error;
			}
		}
		
		if (!dev->bulk_out_endpointAddr &&
			!(endpoint->bEndpointAddress & USB_DIR_IN) &&
			((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
			= = USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk out endpoint */
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
		}
	}
	
	if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
		err("Could not find both bulk-in and bulk-out endpoints");
		goto error;
	}

