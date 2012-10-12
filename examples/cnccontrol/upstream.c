#include "upstream.h"
#include "pressanykey/pressanykey.h"
#include "pressanykey_usb/usb.h"
#include "pressanykey_usb/hid.h"
#include "cnctypes.h"
#include "state.h"
#include "cmdqueue.h"

#define IN_REPORT_SIZE sizeof(ResponseStruct)
#define OUT_REPORT_SIZE sizeof(CommandStruct)

#define POLL_INTERVAL 3

	
	
	
// USB data handlers
uint16_t returnStatus(USB_Device_Struct* device,
							const USBHID_Behaviour_Struct* behaviour,
							USB_HID_REPORTTYPE reportType,
							uint8_t reportId);

void receiveCommand(USB_Device_Struct* device,
						 const USBHID_Behaviour_Struct* behaviour,
						 USB_HID_REPORTTYPE reportType,
						 uint8_t reportId,
						 uint16_t len);


const uint8_t deviceDescriptor[] = {
	0x12,							//bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,				//bDescriptorType: usb device descriptor
	0x00, 0x02,						//bcdUSB: 0200 (Little Endian) - USB 2.0 compliant
	USB_CLASS_DEVICE,				//bDeviceClass: Device class (0 = interfaces specify class)
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

const uint8_t reportDescriptor[] = {
	0x06,I16_TO_LE_BA(0xff01),		// Usage Page (Vendor-defined 1)
	0x09,0x01,			//usage: vendor 1
	0xa1,0x01,			//start of app collection
	0x15,0x00,			//min: 0
	0x26,0xff,0x00,		//max: 0xff
	0x75,0x08,			//report size: 8 bits
	0x95,IN_REPORT_SIZE,	//report count: # bytes
	0x09,0x01,			//usage: vendor usage 1
	0x81,0x02,			//input (data, variable, absolute)
	0x95,OUT_REPORT_SIZE,		//report count # out
	0x09,0x02,			//usage: vendor usage 2
	0x91,0x02,			//output (data, variable, absolute)
	0xc0				//end of collection
};

const uint8_t configDescriptor[] = {
	0x09,								//bLength: length of this descriptor in bytes (9)
	USB_DESC_CONFIGURATION,				//bDescriptorType: configuration descriptor
	I16_TO_LE_BA(34),					//wTotalLen: Total length, including attached interface and endpoint descriptors
	0x01,								//bNumInterfaces: Number of interfaces (1)
	0x01,								//bConfigurationValue: Number to set to activate this config
	0x00,								//iConfiguration: configuration string index (0 = not available)
	0x80,								//bmAttributes: Not self-powered, no remote wakeup
	250,								//bMaxPower: 500mA
	
	// Interface 0: HID
	0x09,								//bLength: length of this descriptor in bytes (9)
	USB_DESC_INTERFACE,					//bDescriptorType: configuration descriptor
	0x00,								//bInterfaceNumber: Interface index, 0-based
	0x00,								//bAlternateSetting
	0x01,								//bNumEndpoints: Number of endpoints excluding control endpoint
	USB_CLASS_HID,						//bInterfaceClass: interface class
	0x00,								//bInterfaceSubClass: non-boot device
	0x00,								//bInterfaceProtocol unused for non-boot device
	0x00,								//iInterface: String index (0x00 = not available)
	
	// HID descriptor (inserted between interface and endpoints for HID devices)
	0x09,								//bLength: length of HID descriptor in bytes
	USB_DESC_HID_HID,					//bDescriptorType: HID descriptor
	I16_TO_LE_BA(0x0100),				//bcdHID: version 1.0
	0x09,								//bCountryCode: 9 = German (HID spec 1.11, section 6.2.1 - 0=not supported, 33=US)
	0x01,								//bNumDescriptors: We have one descriptor
	USB_DESC_HID_REPORT,				//bDescriptorType: It is a report descriptor
	I16_TO_LE_BA(sizeof(reportDescriptor)),	//wDescriptorLength: length of report descriptor
	
	// Endpoint descriptor: 1 interrupt in
	0x07,							    //bLength: Length of endpoint descriptor in bytes
	USB_DESC_ENDPOINT,					//bDescriptorType: This is an endpoint descriptor
	0x81,								//bEndpointAddress: Logical endpoint 1, direction IN
	USB_EPTYPE_INTERRUPT,				//bmAttributesL: Interrput endpoint
	I16_TO_LE_BA(IN_REPORT_SIZE),		//wMaxPacketSize
	POLL_INTERVAL						//bInterval: Poll interval in ms
};

const uint8_t languages[] = {
	0x04,								//bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	I16_TO_LE_BA(0x0409)				//wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};

const uint8_t manufacturerName[] = {
	0x22,								//bLength: length of this descriptor in bytes (34)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'P',0,'r',0,'e',0,'s',0,'s',0,' ',0,'A',0,'n',0,'y',0,' ',0,'K',0,'e',0,'y',0,' ',0,'U',0,'G',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t deviceName[] = {
	24,									//bLength: length of this descriptor in bytes (24)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,'0',0,'x',0,'C',0,'N',0,'C' //bString[]: String (UTF16LE, not terminated)
};

const uint8_t serialName[] = {
	0x0a,								//bLength: length of this descriptor in bytes (10)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'V',0,'1',0,'.',0,'0',0				//bString[]: String (UTF16LE, not terminated)
};

// USB HID state
uint8_t inBuffer[IN_REPORT_SIZE];
uint8_t outBuffer[OUT_REPORT_SIZE];
uint8_t currentProtocol;
uint8_t idleValue;
USB_Device_Struct usbDevice;

const USBHID_Behaviour_Struct hidBehaviour = {
	MAKE_USBHID_BASE_BEHAVIOUR,
	0,
	configDescriptor+18,
	reportDescriptor,
	sizeof(reportDescriptor),
	returnStatus,
	receiveCommand,
	inBuffer,
	outBuffer,
	&idleValue,
	&currentProtocol
};

const USB_Device_Definition usbDeviceDefinition = {
	deviceDescriptor,
	1,
	{ configDescriptor },
	4,
	{ languages, manufacturerName, deviceName, serialName },
	1,
	{ (USB_Behaviour_Struct*)(&hidBehaviour) }
};

uint32_t tickCounter;

void Upstream_Init() {
	USB_Init(&usbDeviceDefinition, &usbDevice);
	tickCounter;
}

void Upstream_Start() {
	USB_SoftConnect(&usbDevice);
}


void Upstream_Tick() {
	tickCounter++;
	if (tickCounter > ((HEARTBEAT_HZ * (POLL_INTERVAL+1)) / 1000 + 1)) {
		tickCounter = 0;
		USBHID_PushReport (&usbDevice, &hidBehaviour, USB_HID_REPORTTYPE_INPUT, 0);
	}
}

uint16_t returnStatus(USB_Device_Struct* device,
						 const USBHID_Behaviour_Struct* behaviour,
						 USB_HID_REPORTTYPE reportType,
						 uint8_t reportId) {
	int i;
	for (i=0;i<NUM_AXES;i++) {
		((ResponseStruct*)inBuffer)->currentPos[i] = currentPosition[i];
	}
	((ResponseStruct*)inBuffer)->stateFlags = stateFlags;
	((ResponseStruct*)inBuffer)->freeSlots = CQ_EmptySlots(&(((ResponseStruct*)inBuffer)->lastTransactionId));
	return IN_REPORT_SIZE;
}

void receiveCommand(USB_Device_Struct* device,
					const USBHID_Behaviour_Struct* behaviour,
					USB_HID_REPORTTYPE reportType,
					uint8_t reportId,
					uint16_t len) {
	if (reportId != 0) return;
	if (reportType != USB_HID_REPORTTYPE_OUTPUT) return;
	CQ_AddCommand((CommandStruct*)outBuffer);
	
}



