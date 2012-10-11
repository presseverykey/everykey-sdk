#include "pressanykey/pressanykey.h"
#include "pressanykey_usb/cdc.h"

const uint8_t deviceDescriptor[] = {
	18,								//bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,				//bDescriptorType: usb device descriptor
	I16_TO_LE_BA(0x0200),			//bcdUSB - USB 2.0
	USB_CLASS_CDC,					//bDeviceClass: CDC
	0x00,							//bDeviceSubClass: Device subclass (must be 0 if bDeviceClass is 0)
	0x00,							//bDeviceProtocol: 0 for no specific device-level protocols
	USB_MAX_COMMAND_PACKET_SIZE,	//bMaxPacketSize0: Max packet size for control endpoint
	I16_TO_LE_BA(0x1234),           //idVendor: 16 bit vendor id
	I16_TO_LE_BA(0x5678),           //idProduct: 16 bit product id
	I16_TO_LE_BA(0x0100),			//bcdDevice: Device release version
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
	0x1a,							//bLength: length of this descriptor in bytes (26)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,'0',0,'x',0,' ',0,'C',0,'D',0,'C',0
};

const uint8_t serialName[] = {
	0x0a,							//bLength: length of this descriptor in bytes (10)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'V',0,'1',0,'.',0,'0',0			//bString[]: String (UTF16LE, not terminated)
};

const uint8_t configDescriptor[] = {
	9,								//bLength: length of this descriptor in bytes (9)
	USB_DESC_CONFIGURATION,			//bDescriptorType: configuration descriptor
	I16_TO_LE_BA(60),				//wTotalLen: Total length, including attached interface and endpoint descriptors
	0x02,							//bNumInterfaces: Number of interfaces (1)
	0x01,							//bConfigurationValue: Number to set to activate this config
	0x00,							//iConfiguration: configuration string index (0 = not available)
	0x80,							//bmAttributes: Not self-powered, no remote wakeup
	0x32,							//bMaxPower: Max power in 2mA steps (0x32 = 50 = 100mA)
	
	//interface 0: Communication interface
	9,								//bLength: length of this descriptor in bytes (9)
	USB_DESC_INTERFACE,				//bDescriptor type: constant indicating that this is an interface descriptor
	0x00,							//bInterfaceNumber: Interface index, 0-based
	0x00,							//bAlternateSetting
	0x01,							//bNumEndpoints: One interrupt endpoint
	USB_CDC_INTERFACE_COMMUNICATION_INTERFACE,	//bInterfaceClass: CDC comm
	USB_CDC_SUBCLASS_ABSTRACT_CONTROL_MODEL,	//bInterfaceSubClass
	USB_CDC_CI_PROTOCOL_NONE,		//bInterfaceProtocol
	0x00,							//iInterface: String index (0x00 = not available)
	
	//Header functional descriptor
	5,								//bLength
	0x24,							//bDescriptorType: Class-specific (0x20) + interface (0x04)
	USB_CDC_HEADER_FUNC_DESC,		//bDescriptorSubtype
	I16_TO_LE_BA(0x0110),			//bcdCDC
	
	//Call mgmgt func desc
	5,								//bLength
	0x24,							//bDescriptorType: Class-specific (0x20) + interface (0x04)
	USB_CDC_CALL_MGMT_FUNC_DESC,	//bDescriptorSubtype
	0x01,							//bmCapabilities (call management is done by device)
	0x01,							//bDataInterface (our associated data interface)

	//ACM desc
	4,								//bLength
	0x24,							//bDescriptorType: Class-specific (0x20) + interface (0x04)
	USB_CDC_ABSTRACT_CONTROL_MODEL_FUNC_DESC,	//bDescriptorSubtype
	0x02,							//bmCapabilities (none) (set control line state, set/get line coding)
	
	//Union func desc
	5,								//bLength
	0x24,							//bDescriptorType: Class-specific (0x20) + interface (0x04)
	USB_CDC_UNION_FUNC_DESC,		//bDescriptorSubtype
	0x00,							//bMasterInterface - this is the master interface
	0x01,							//bSlaveInterface0 - this is the first (and only) slave interface
	
	//Notification endpoint (physical index: 5)
	7,								//bLength
	USB_DESC_ENDPOINT,				//bDescriptorType
	0x82,							//in endpoint 2
	USB_EPTYPE_INTERRUPT,			//bmAttributes
	I16_TO_LE_BA(0x0010),			//wMaxPacketSize
	2,								//bInterval: 2ms
	
	//interface 1: Data interface
	9,								//bLength: length of this descriptor in bytes (9)
	USB_DESC_INTERFACE,				//bDescriptor type: constant indicating that this is an interface descriptor
	0x01,							//bInterfaceNumber: Interface index, 0-based
	0x00,							//bAlternateSetting
	0x02,							//bNumEndpoints: Number of endpoints excluding control endpoint
	USB_CDC_INTERFACE_DATA_INTERFACE,	//bInterfaceClass: CDC data
	0x00,							//bInterfaceSubClass
	USB_CDC_DI_PROTOCOL_NONE,		//bInterfaceProtocol
	0x00,							//iInterface: String index (0x00 = not available)
	
	//endpoint 1: data out (physical index: 2)
	7,								//bLength
	USB_DESC_ENDPOINT,				//bDescriptorType
	0x01,							//bEndpointAddres: endpoint 1 (out)
	USB_EPTYPE_BULK,				//bmAttributes
	I16_TO_LE_BA(64),				//wMaxPacketSize
	0,								//bInterval

	//endpoint 2: data in (physical index: 3)
	7,								//bLength
	USB_DESC_ENDPOINT,				//bDescriptorType
	0x81,							//bEndpointAddres: endpoint 1 (in)
	USB_EPTYPE_BULK,				//bmAttributes
	I16_TO_LE_BA(64),				//wMaxPacketSize
	0								//bInterval
	
};

const USBCDC_Behaviour_Struct cdcBehaviour = {
	MAKE_USBCDC_BASE_BEHAVIOUR
};

const USB_Device_Definition usbDefinition = {
	deviceDescriptor,
	1,
	{ configDescriptor },
	4,
	{ languages, manufacturerName, deviceName, serialName },
	0,
	{ (USB_Behaviour_Struct*)(&cdcBehaviour) }
};
	
USB_Device_Struct usbDevice;

void main(void) {	
	USB_Init(&usbDefinition, &usbDevice);
	USB_SoftConnect(&usbDevice);
}
