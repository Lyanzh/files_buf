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

### USB endpoints described in the kerne ###

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

USB interfaces handle only one type of a USB logical connection, such as a mouse, a keyboard, or a audio stream.

Some USB devices have multiple interfaces, such as a USB speaker that might consist of two interfaces: a USB keyboard for the buttons and a USB audio stream.

a USB interface represents basic functionality, each USB driver controls an interface.

described in the kernel with the struct `usb_interface` structure.

This structure is what the USB core passes to USB drivers and is what the USB driver then is in charge of controlling.

 important fields:

- struct usb_host_interface *altsetting

	containingall of the alternate settings that may be selected for this interface.

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

//to be continue...

### USB Urbs ###

A urb(USB request block) is used to send or receive data to or from a specific USB endpoint on a specific USB device in an asynchronous manner.

described with the struct urb structure.

Every endpoint in a device can handle a queue of urbs.

