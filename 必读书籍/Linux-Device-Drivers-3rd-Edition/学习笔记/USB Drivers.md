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

