#include "anykey/anykey.h"
#include "anykey_usb/usb.h"

const uint8_t deviceDescriptor[] = {
	0x12,							//bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,				//bDescriptorType: usb device descriptor
	0x00, 0x02,						//bcdUSB: 0200 (Little Endian) - USB 2.0 compliant
	0x00,							//bDeviceClass: Device class (0 = interfaces specify class)
	0x00,							//bDeviceSubClass: Device subclass (must be 0 if bDeviceClass is 0)
	0x00,							//bDeviceProtocol: 0 for no specific device-level protocols
	USB_MAX_COMMAND_PACKET_SIZE,	//bMaxPacketSize0: Max packet size for control endpoint
	0x34, 0x12,                     //idVendor: 16 bit vendor id
	0x78, 0x56,                     //idProduct: 16 bit product id
	0x00, 0x01,						//bcdDevice: Device release version
	0x01,                           //iManufacturer: Manufacturer string index
	0x02,                           //iProduct: Product string index
	0x03,                           //iSerialNumber: Serial number string index
	0x01                            //bNumConfigurations: Number of configurations
};

const uint8_t languages[] = {
	0x04,							//bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	0x09,0x04						//wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};

const uint8_t manufacturerName[] = {
	0x22,							//bLength: length of this descriptor in bytes (34)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'P',0,'r',0,'e',0,'s',0,'s',0,' ',0,'A',0,'n',0,'y',0,' ',0,'K',0,'e',0,'y',0,' ',0,'U',0,'G',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t deviceName[] = {
	0x12,							//bLength: length of this descriptor in bytes (18)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,'0',0,'x',0			//bString[]: String (UTF16LE, not terminated)
};

const uint8_t serialName[] = {
	0x0a,							//bLength: length of this descriptor in bytes (10)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'V',0,'1',0,'.',0,'0',0			//bString[]: String (UTF16LE, not terminated)
};

const uint8_t configDescriptor[] = {
	0x09,							//bLength: length of this descriptor in bytes (9)
	USB_DESC_CONFIGURATION,			//bDescriptorType: configuration descriptor
	0x12, 0x00,						//wTotalLen: Total length, including attached interface and endpoint descriptors
	0x01,							//bNumInterfaces: Number of interfaces (1)
	0x01,							//bConfigurationValue: Number to set to activate this config
	0x00,							//iConfiguration: configuration string index (0 = not available)
	0x80,							//bmAttributes: Not self-powered, no remote wakeup
	0x32,							//bMaxPower: Max power in 2mA steps (0x32 = 50 = 100mA)
	
	//interface 0
	0x09,							//bLength: length of this descriptor in bytes (9)
	USB_DESC_INTERFACE,				//bDescriptor type: constant indicating that this is an interface descriptor
	0x00,							//bInterfaceNumber: Interface index, 0-based
	0x00,							//bAlternateSetting
	0x00,							//bNumEndpoints: Number of endpoints excluding control endpoint
	0xff,							//bInterfaceClass: interface class (0xff = vendor specific)
	0x00,							//bInterfaceSubClass
	0xff,							//bInterfaceProtocol (0xff = vendor specific)
	0x00							//iInterface: String index (0x00 = not available)
	
	//add endpoint descriptors here...
};
	
const USB_Device_Definition myUSBDeviceDefinition = {
	deviceDescriptor,
	1,
	{ configDescriptor },
	4,
	{languages, manufacturerName, deviceName, serialName },
	0,
	{ NULL }
};

USB_Device_Struct myUSBDevice;

void main(void) {
	USB_Init(&myUSBDeviceDefinition, &myUSBDevice);
	USB_SoftConnect(&myUSBDevice);
}
