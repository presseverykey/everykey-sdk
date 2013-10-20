

// This header contains all the "boilerplate" data structures that
// you'll need to implement a usb midi device. USB device descriptors
// are notoriously long, convoluted and complicated so we decided to
// keep them seperate so you can concentrate on the code you'll actually
// need to implement. 
// In case you just want to get MIDI working, you can use these
// describtors in your project as is. In case you want to program a
// custom device, you'll need to dive in here and change things around a
// bit... The describtors below are the sample describtors stolen
// directly from the USB Audio Spec

// Further Reading:
// - The USB MIDI class is a subclass of the Audio class, which is described in
// this document, which contains some of the necessary constants:
//   http://www.usb.org/developers/devclass_docs/audio10.pdf
// - The particulars of the USB MIDI class itself are contained in their own 
// specification, which you can find here:
//   http://www.usb.org/developers/devclass_docs/midi10.pdf
// - USB per se is a beast of a protocol. In case you'd like to
// understand how it works, we can recommend the following two resources
// as a good starting point to learn about the internal workings:
//   http://www.beyondlogic.org/usbnutshell/usb1.shtml
//   http://www.usbmadesimple.co.uk/ums_1.htm
// - The USB standards (long, detailed, boring) are freely available at
// usb.org in case you want to consult a definative resource.
// - How to use the USB capabilities of the Anykey is detailed in 
// Chapter 10 of the LPC1343 user manual available here:
//   http://www.nxp.com/documents/user_manual/UM10375.pdf

#ifndef _USB_MIDI_BOILERPLATE
#define _USB_MIDI_BOILERPLATE

#include "anykey_usb/midi.h"


// USB devices identify themselves to hosts by providing a
// DeviceDescriptor, which itself contains a number of
// ConfigurationDescriptors. Each ConfigurationDescriptor contains one
// or more InterfaceDescriptors which in turn contain Endpoint
// Desciptors (IN and OUT)
// These datastructures are defined below.

// The DeviceDescriptor contains metadata for the host: USB Version,
// maximum size of packets that can be handled by the device, etc. and
// finally the number of Configurations (next section) the device
// supports.
const uint8_t midi_deviceDescriptor[] = {

	0x12,                           //bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,                //bDescriptorType: usb device descriptor
	I16_TO_LE_BA(0x0110),           //bcdUSB: 0110 (Little Endian) - USB 1.1 spec
	USB_CLASS_DEVICE,               //bDeviceClass: Device class (0 = interfaces specify class)
	0x00,                           //bDeviceSubClass: Device subclass (must be 0 if bDeviceClass is 0)
	0x00,                           //bDeviceProtocol: 0 for no specific device-level protocols
	USB_MAX_COMMAND_PACKET_SIZE,    //bMaxPacketSize0: Max packet size for control endpoint
	I16_TO_LE_BA(0x5678),           //idVendor: 16 bit vendor id
	I16_TO_LE_BA(0x1234),           //idProduct: 16 bit product id
	I16_TO_LE_BA(0x0100),           //bcdDevice: Device release version
	0x01,                           //iManufacturer: Manufacturer string index
	0x02,                           //iProduct: Product string index
	0x03,                           //iSerialNumber: Serial number string index
	0x01                            //bNumConfigurations: Number of configurations
};

// First off, note that this midi_configDescriptors array also contains the
// Interface and Endpoint descriptions for the device...
// The Configuration Descriptor itself only contains information about
// how the device is powered and the number of interfaces the
// configuration contains..
// In this example, we only provide a single configuration for
// the host. A device could offer different configurations for the host
// to choose from, e.g. one bus powered and on externally powered
// configuration.

// All interface descriptors, regardless of their USB class start with 9
// standard bytes identifying the usb class of the interface and it's
// endpoints. These bytes are typically followed by USB class specific
// information and finally by descriptions of the endpoints of the
// interface.

// We define two interfaces: 
//   - an AUDIO_CONTROL interface
// which doesn't contain any endpoints but references a
//   - MIDI_STREAMING interface
// AUDIO_CONTROL and MIDI_STREAMING are subclasses of the USB AUDIO
// class.

// The MIDI_STREAMING interface itself describes a number of MIDI Jacks
// which are connected to USB Enpoints. Jacks are classified as EMBEDDED
// or EXTERNAL, embedded jacks are virtual jacks to USB, while external
// jacks represent physical MIDI connectors. Furthermore, jacks are
// labeled IN and OUT, this is relative to the Anykey so an IN jack
// receives data while OUT jacks are used to send data. 

// Now things become confusing, because the USB interface descriptor
// contains the description of the USB endpoints. These are also labeled
// IN and OUT, but in the USB world  IN and OUT are always relative to
// the host. So the OUT endpoint is the one we'll be receiving data on
// and the IN endpoint is used to send data. As a consequence, the USB
// OUT endpoint is associated with the MIDI embedded IN jack and the IN
// endpoint with the MIDI embedded OUT jack.

// The association between MIDI jack and USB endpoint is made via
// jackIDs which are referenced from both the MIDI_STREAMING interface
// descriptor and the Endpoint descriptor.

// To make things even more confusing, the USB MIDI spec only calls this
// association "jack" in the descriptor and "cable" everywhere else.

#define DATA_OUT_ENDPOINT_LOGICAL 0x01
#define DATA_OUT_ENDPOINT_PHYSICAL 2
#define DATA_IN_ENDPOINT_LOGICAL 0x81
#define DATA_IN_ENDPOINT_PHYSICAL 3


// see USB Device Class Definition for Audio Devices, Appendix A for 
// constants.
#define USB_AUDIO_INTERFACE_CLASS_CODE   0x01
#define USB_AUDIO_SUBCLASS_AUDIOCONTROL  0x01
#define USB_AUDIO_SUBCLASS_MIDISTREAMING 0x03

const uint8_t midi_configDescriptor[] = {
	//Config descriptor
	9,                              //bLength
	USB_DESC_CONFIGURATION,         //bDescriptorType
	I16_TO_LE_BA(101),              //wTotalLength (configDesc complete)
	2,                              //bNumInterfaces
	1,                              //bConfigurationValue
	0,                              //iConfiguration
	0x80,                           //bmAttributes : bus powered
	50,                             //bMaxPower : 100mA

	//Standard Audio Class Interface Descriptor
	0x09,                           // bLength - ￼Size of this descriptor, in bytes.
	0x04,                           // bDescriptorType - INTERFACE descriptor.
	0x00,                           // bInterfaceNumber - Index of this interface.
	0x00,                           // bAlternateSetting - Index of this setting.
	0x00,                           // bNumEndpoints - 0 endpoints.
	USB_AUDIO_INTERFACE_CLASS_CODE, // bInterfaceClass - AUDIO.
	USB_AUDIO_SUBCLASS_AUDIOCONTROL,// bInterfaceSubclass - AUDIO_CONTROL.
	0x00,                           // bInterfaceProtocol - ￼Unused.
	0x00,                           // iInterface - ￼Unused.

	//Class-specific Audio Class Interface Descriptor
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - 
	0x01,                   // bDescriptorSubtype - HEADER subtype.
	I16_TO_LE_BA(0x0100),   //bcdADC - Revision of class specification - 1.0
	I16_TO_LE_BA(0x0009),   // wTotalLength - Total size of class specific descriptors.
	0x01,                   // bInCollection - Number of streaming interfaces.
	0x01,                   // baInterfaceNr(1) - MIDIStreaming interface 1 belongs to this AudioControl interface.

	//Standard MIDI Streaming Interface Descriptor
	0x09,                            // bLength - Size of this descriptor, in bytes.
	0x04,                            // bDescriptorType - INTERFACE descriptor.
	0x01,                            // bInterfaceNumber - Index of this interface.
	0x00,                            // bAlternateSetting - Index of this alternate setting.
	0x02,                            // bNumEndpoints - 2 endpoints.
	USB_AUDIO_INTERFACE_CLASS_CODE,  // bInterfaceClass - AUDIO.
	USB_AUDIO_SUBCLASS_MIDISTREAMING,// bInterfaceSubclass - MIDISTREAMING.
	0x00,                            // bInterfaceProtocol - Unused.
	0x00,                            // iInterface - Unused.

	// Class-specific MIDI Streaming Interface Descriptor
	0x07,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x01,                   // bDescriptorSubtype - MS_HEADER subtype.
	I16_TO_LE_BA(0x0100),   // BcdADC - ￼Revision of this class specification.
	I16_TO_LE_BA(65),   	// wTotalLength - ￼Total size of class-specific descriptors. Doc says all elements and jacks, but example indicates also the endpoints

	//MIDI IN Jack Descriptor (Embedded)
	0x06,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x02,                   // bDescriptorSubtype - MIDI_IN_JACK subtype.
	0x01,                   // bJackType - EMBEDDED.
	0x01,                   // bJackID - ID of this Jack.
	0x00,                   // iJack - Unused.

	//MIDI Adapter MIDI IN Jack Descriptor (External)
	0x06,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x02,                   // bDescriptorSubtype - MIDI_IN_JACK subtype.
	0x02,                   // bJackType - EXTERNAL .
	0x02,                   // bJackID - ID of this Jack.
	0x00,                   // iJack - Unused.

	//MIDI OUT Jack Descriptor (Embedded)
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x03,                   // bDescriptorSubtype - MIDI_OUT_JACK subtype.
	0x01,                   // bJackType - EMBEDDED.
	0x03,                   // bJackID - ID of this Jack.
	0x01,                   // bNrInputPins - Number of Input Pins of this Jack.
	0x02,                   // BaSourceID(1) - ID of the Entity to which this Pin is connected.
	0x01,                   // BaSourcePin(1) - Output Pin number of the Entity to which this Input Pin is connected.
	0x00,                   // iJack - Unused.

	//MIDI Adapter MIDI OUT Jack Descriptor (External)
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x03,                   // bDescriptorSubtype - MIDI_OUT_JACK subtype.
	0x02,                   // bJackType - EXTERNAL.
	0x04,                   // bJackID - ID of this Jack.
	0x01,                   // bNrInputPins - Number of Input Pins of this Jack.
	0x01,                   // BaSourceID(1) - ID of the Entity to which this Pin is connected.
	0x01,                   // BaSourcePin(1) - Output Pin number of the Entity to which this Input Pin is connected.
	0x00,                   // iJack - Unused.

	//Standard Bulk OUT Endpoint Descriptor
	0x09,                     // bLength - Size of this descriptor, in bytes.
	0x05,                     // bDescriptorType - ENDPOINT descriptor.
	DATA_OUT_ENDPOINT_LOGICAL,// bEndpointAddress - OUT Endpoint 1.
	0x02,                     // bmAttributes - Bulk, not shared.
	I16_TO_LE_BA(0x0040),     // wMaxPacketSize - 64 bytes per packet.
	0x00,                     // bInterval - Ignored for Bulk. Set to zero.
	0x00,                     // bRefresh - Unused.
	0x00,                     // bSynchAddress - Unused.

	//Class-specific MS Bulk OUT Endpoint Descriptor
	0x05,                   // bLength - Size of this descriptor, in bytes.
	0x25,                   // bDescriptorType - CS_ENDPOINT descriptor
	0x01,                   // bDescriptorSubtype - MS_GENERAL subtype.
	0x01,                   // bNumEmbMIDIJack - Number of embedded MIDI IN Jacks.
	0x01,                   // BaAssocJackID(1) - ID of the Embedded MIDI IN Jack.

	//Standard Bulk IN Endpoint Descriptor
	0x09,                    // bLength - Size of this descriptor, in bytes.
	0x05,                    // bDescriptorType - ENDPOINT descriptor.
	DATA_IN_ENDPOINT_LOGICAL,// bEndpointAddress - IN Endpoint 1.
	0x02,                    // bmAttributes - Bulk, not shared.
	I16_TO_LE_BA(0x0040),    // wMaxPacketSize - 64 bytes per packet.
	0x00,                    // bInterval - Ignored for Bulk. Set to zero.
	0x00,                    // bRefresh - Unused.
	0x00,                    // bSynchAddress - Unused.

	//Class-specific MS Bulk IN Endpoint Descriptor
	0x05,                   // bLength - Size of this descriptor, in bytes.
	0x25,                   // bDescriptorType - CS_ENDPOINT descriptor
	0x01,                   // bDescriptorSubtype - MS_GENERAL subtype.
	0x01,                   // bNumEmbMIDIJack - Number of embedded MIDI OUT Jacks.
	0x03                    // BaAssocJackID(1) - ID of the Embedded MIDI OUT Jack.;
};

// Finally, the decribtors sent to the host may contain "string
// describtors" which provide human readable information about the
// device...

const uint8_t midi_languages[] = {
	0x04,                //bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,     //bDescriptorType: string descriptor
	I16_TO_LE_BA(0x0409) //wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};
	
const uint8_t midi_manufacturerName[] = {
	0x22,             //bLength: length of this descriptor in bytes (34)
	USB_DESC_STRING,  //bDescriptorType: string descriptor
	'P',0,'r',0,'e',0,'s',0,'s',0,' ',0,'A',0,'n',0,'y',0,' ',0,'K',0,'e',0,'y',0,' ',0,'U',0,'G',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t midi_deviceName[] = {
	0x14,             //bLength: length of this descriptor in bytes (20)
	USB_DESC_STRING,  //bDescriptorType: string descriptor
	'P',0,'S',0,'/',0,'2',0,'-',0,'M',0,'I',0,'D',0,'I',0			//bString[]: String (UTF16LE, not terminated)
};

const uint8_t midi_serialName[] = {
	0x0a,             //bLength: length of this descriptor in bytes (10)
	USB_DESC_STRING,  //bDescriptorType: string descriptor
	'V',0,'1',0,'.',0,'0',0				//bString[]: String (UTF16LE, not terminated)
};



#endif
