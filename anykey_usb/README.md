anykey_usb
===============

a free USB software stack for the LPC1343

What is it?
-----------

USB is a rather complex protocol with multiple protocol layers. Microcontrollers with on-board USB typically contain dedicated hardware for  handling the lower layers (some devices include a PHY that contains the lowest protocol layer, some need additional hardware). The USB hardware is typically controlled via some hardware registers, a Serial Interface Engine (SIE) that takes care of timing-critical portions of data transfer in hardware, some endpoint data buffers and one or multiple interrupts to be notified about USB events.

This hardware is required and helpful for implementing USB. However, itÄs not the complete USB protocol stack. The remaining protocol stack needs to be implemented in software: Controlling data transfers on a higher level, sequencing them into USB commands (responding to Setup packets), and implementing required USB commands.

The LPC1343 comes with built-in USB implementations for the Mass Storage and Human Interface Device classes in ROM. However, these are very limited and not extendable, so if you want to implement more advanced USB devices, you have to implement the missing USB protocol layers yourself.

anykey_usb fills exactly this gap. The USB hardware implements the lower protocol layers, anykey_usb implements the higher layers. Your application code just adds the application layer on top.

To get an idea of what it can do, have a look at the examples: usbraw implements a basic, raw USB device, usbkeyboard implements a USB keyboard that types a sentence at the click of a button, usbaudio implements a USB sound card with a LED level meter and a morse tone button, cdcleetifier implements a simple virtual serial port with a loopback device that modifies text to a N3RD1SH slang.

Note that anykey_usb does _NOT_ free you to familiarize yourself with concepts of USB. Some tasks, such as writing device and configuration descriptors, are still left to do for you. "USB in a nutshell" is a good entry point to get started with USB.

Design goals
------------

If you look at the code, some parts might seem a bit odd at a first glance. Understanding the design may be easier if you know the design goals of the library:

- Modular: For example, USB class implementations are independent and the library has no knowledge about them. If you don't need a specific USB class implementation, you can simple remove it to save code memory. It is easy to add other USB class or custom device behaviours. The library is written in an object-oriented fashion (with the limited options of plain C on an embedded platform).

- Simple to use, full control: anykey_usb allows you to get a simple raw USB device running with just a couple lines of code. In addition, it provides several USB class implementations. For example, it's simple to build a USB keyboard or mouse, a USB soundcard or a virtual serial port. If you want to build your custom USB device or add other USB classes, you simply attach your own behaviour without having to modify the library.

- Avoiding globals: Although the LPC1343 only has one USB device, the library was written with multiple USB devices in mind. All USB calls keep a reference to their respective USB connection, so that porting to architectures with multiple USB engines should be fairly simple. Currently, there is only one global variable to obtain the device structures from interrupts - only this part needs to be rewritten for MCUs with multiple USB devices on board.

- Pass-in storage: anykey_usb does not allocate or reserve memory on its own (with the single exception mentioned above). All resources are passed in by the user application, so you are free to use your memory as you want.

- Separation of static and dynamic storage: USB Devices usually have static  properties (such as the device descriptor etc.) and dynamic runtime state. anykey_usb maintains separate structures for these, so that static properties can reside in Flash memory, facilitating initialization and saving RAM. 

- Library independence: Right now, anykey_usb is an optional addition to the anykey runtime library for the LPC1343. However, it has only few references to the anykey library (mainly the hardware register definitions in memorymap.h and some basic types found in types.h). It should be fairly straightforward to make this library completely standalone.

How it works - Architecture
---------------------------

### file structure

As mentioned above, the code is quite modular: usb.h and usb.c contain the core USB driver that talks to the USB hardware and implements common, required USB device behaviour. It contains a plugin mechanism that allows USB class or custom behaviour to be added to the device.

USB class bahaviours are implemented in usbaudio, hid and cdc (each comes with a .h header and .c implementation file). They are independent of each other and the core USB implementation does not require them, so they can be extended, modified or deleted. Note that the class implementations are not necessarily complete - for example, the CDC implementation just implements a virtual serial port (Abstract Control Model). Most of the USB device specifications are very generic and complex, so we have limited them to a degree that works for most typical uses, but is still easy to use. If you want to go beyond that, feel free to extend them.

Header files ending with "spec.h" contain definitions of the respective USB specification (most of them are not complete, they just contain the portions of the spec that are required the purposes of the library). The files reflect the specification documents found at USB.org.

There is a small number of helper and utility files: ringbuffer.h/.c implements a simple byte fifo buffer, used in the CDC class implementation. keyboard.c and keyboard.h is a special case of a HID device, giving you an  even easier to use, pre-build USB keyboard implementation.

### core types

anykey_usb is build around two basic concepts (you would call them classes in an object oriented world): USB DEVICE and USB BEHAVIOUR.

A USB_DEVICE is associated to a specific USB hardware / port. Since the LPC1343 only features one USB hardware, there will only be one USB_DEVICE object. A USB DEVICE consists of a USB_Device_Struct (dynamic runtime state, in RAM) and a USB_Device_Definition (static description of the USB device, usually in Flash memory). The device struct has a reference to the device definition, so you need to pass the reference to the device struct.

A USB_BEHAVIOUR specifies a specific behaviour of a USB device (the term "Behaviour" is not specified in the USB spec, it's a concept of anykey_usb). a USB BEHAVIOUR allows you to attach capabilities to your USB DEVICE. For example, our USB class implementations (HID, CDC and USB Audio) are based on behaviours. If you want your USB device to have HID capabilities, you simply add a HID behaviour to it. Adding multiple behaviours allows you to build composite devices, such as, for example, a USB audio interface with a HID-based volume control.

USB behaviours are meant to be "subclassed": A USB behaviour consists of a USB_Behaviour_Struct, containing some callbacks that allows the behaviour to implement its functionality. A specific behaviour typically needs more information and some state storage, so it typically defines its own behaviour structure (e.g. a USBHID_Behaviour_Struct for a HID behaviour). It is essential that these subclassed / extended structures start with a USB_Behaviour_Struct, so that the USB core can typecast it and call your callbacks. This way, the USB core does not need to know specifics of the behaviour, it will simply call the provided callbacks. Since the behaviour knows itself, it can safely typecast the provided behavour struct back to its own type. Have a look at the provided behaviours to see how it works in detail.

How to use
----------

### Use a device class

Using one of the provided USB class implementations is straightforward:

- If your behaviour needs memory in RAM, you allocate appropriate RAM (typically by declaring non-const global variables, not initializing them). 

- Then you initialize a behaviour struct of the behaviour you want to use, e.g. a USBHID_Behaviour_Struct for a HID device (you can do this statically with a const declaration, there's usually no need to change it during runtime. There are helper macros for fillng the basic behaviour struct, have a look at the examples). The behaviour typically also contains a couple of callbacks that allows your device to do its job (e.g. for HID, the callbacks allow you to read and write reports).

- Then you allocate a USB_Device_Struct (e.g. a non-const global variable, you don't need to initialize the values) and initialize a USB_Device_Definition, filling all fields with the description of your device (again, you can use a const global struct here). Make sure to add your behaviour struct to the device definition.

- Call USB_Init() and pass your USB device definition and device struct.

- Call USB_SoftConnect() once your device is ready to run.

### Build a composite USB device

Building a composite USB device is basically the same as above. The only difference is that you instantiate multiple USB behaviours (of the same or different types) and attach all of them to the USB device.

### Build your own USB behaviour

If the provided USB class implementations don't fit your needs or if you want to implement some custom USB protocol, you may do so by building your own USB behaviour. You can implement up to five callbacks to modify or extend the way the USB driver reacts to requests or data.

You may use an unaltered USB_Behaviour_Struct to do this. However, in most cases, it is useful to define your own behaviour structure with a USB_Behaviour_Struct as the first entry. Have a look at the provided class implementation to see how it is done.

Note that USB behaviours are not limited to extending the device's behaviour. Instead, they can also intercept requests to the underlying core driver. The driver core will first offer the request to all attached behaviours. If one of them signals that the request was handled, it will not process it further. So, for example, if your behaviour states that it handled a SET_CONFIGURATION request, the core driver will not take care of configuring the device any more. This approach offers more flexibility, but requires behaviours to act responisbly as one bad behaviour can break the whole device.
