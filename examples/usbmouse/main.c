#include "anykey/anykey.h"
#include "anykey_usb/hid.h"
#include "anypio.h"

/** We assume there's a button (pull-low) at BUTTON_PIN and potentiometers (0 - 3.3V) at X and Y.
This example turns them into a USB mouse */

#define BUTTON PIN_1_1	
#define X PIN_0_11
#define Y PIN_1_0

#define AVG_COUNT 30
#define X_IDLE 512
#define Y_IDLE 512
#define SENS_SHIFT 4

/** Our current state */

/** the first cycles are used to estimate the idle position of the potentiometers */
int avgCounter = 0;
int avgX = 0;
int avgY = 0;

/** Theses are the current values of the axes and the button */
int x;
int y;
bool button;

/* USB HID FROM HERE */

/** these variables are needed by the Mouse implementation. USB peripherals don't allocate
 memory by themselves, so we have to give some memory to them. They usually handle all related stuff. */

USB_Device_Struct mouse_device;

#define IN_REPORT_LENGTH 3
#define OUT_REPORT_LENGTH 3

uint8_t inBuffer[IN_REPORT_LENGTH];	//reports from device to host
uint8_t outBuffer[3];				//reports from host to device
uint8_t idleValue;
uint8_t currentProtocol;

/** generate an IN report. We use one button, x and y (rel) */
uint16_t inReportHandler (USB_Device_Struct* device,
						  const USBHID_Behaviour_Struct* behaviour,
						  USB_HID_REPORTTYPE reportType,
						  uint8_t reportId) {
	inBuffer[0] = button ? 0x01 : 0x00;
	inBuffer[1] = (x - avgX) >> SENS_SHIFT;
	inBuffer[2] = (y - avgY) >> SENS_SHIFT;
	return 3;
}

/** parse an OUT report. We just ignore them */
void outReportHandler (USB_Device_Struct* device,
					   const USBHID_Behaviour_Struct* behaviour,
					   USB_HID_REPORTTYPE reportType,
					   uint8_t reportId,
					   uint16_t len) {
}

const uint8_t mouse_deviceDescriptor[] = {
	0x12,				              //bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,				        //bDescriptorType: usb device descriptor
	0x00, 0x02,						          //bcdUSB: 0200 (Little Endian) - USB 2.0 compliant
	USB_CLASS_DEVICE,				        //bDeviceClass: Device class (0 = interfaces specify class)
	0x00,							              //bDeviceSubClass: Device subclass (must be 0 if bDeviceClass is 0)
	0x00,							              //bDeviceProtocol: 0 for no specific device-level protocols
	USB_MAX_COMMAND_PACKET_SIZE,	  //bMaxPacketSize0: Max packet size for control endpoint
	0x34, 0x12,                     //idVendor: 16 bit vendor id
	0x78, 0x56,                     //idProduct: 16 bit product id
	0x00, 0x01,						          //bcdDevice: Device release version
	0x01,                           //iManufacturer: Manufacturer string index
	0x02,                           //iProduct: Product string index
	0x03,                           //iSerialNumber: Serial number string index
	0x01                            //bNumConfigurations: Number of configurations
};

const uint8_t mouse_reportDescriptor[] = {
	0x05, 0x01,							// USAGE_PAGE (Generic Desktop)
	0x09, 0x02,							// USAGE (Mouse)
	0xa1, 0x01,							// COLLECTION (Application)
	0x09, 0x01,							//   USAGE (Pointer)
	0xa1, 0x00,							//   COLLECTION (Physical)
	0x05, 0x09,							//     USAGE_PAGE (Button)
	0x19, 0x01,							//     USAGE_MINIMUM (Button 1)
	0x29, 0x03,							//     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,							//     LOGICAL_MINIMUM (0)
	0x25, 0x01,							//     LOGICAL_MAXIMUM (1)
	0x95, 0x03,							//     REPORT_COUNT (3)
	0x75, 0x01,							//     REPORT_SIZE (1)
	0x81, 0x02,							//     INPUT (Data,Var,Abs)
	0x95, 0x01,							//     REPORT_COUNT (1)
	0x75, 0x05,							//     REPORT_SIZE (5)
	0x81, 0x03,							//     INPUT (Cnst,Var,Abs)
	0x05, 0x01,							//     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,							//     USAGE (X)
	0x09, 0x31,							//     USAGE (Y)
	0x15, 0x81,							//     LOGICAL_MINIMUM (-127)
	0x25, 0x7f,							//     LOGICAL_MAXIMUM (127)
	0x75, 0x08,							//     REPORT_SIZE (8)
	0x95, 0x02,							//     REPORT_COUNT (2)
	0x81, 0x06,							//     INPUT (Data,Var,Rel)
	0xc0,								//   END_COLLECTION
	0xc0 								// END_COLLECTION
};

const uint8_t mouse_configDescriptor[] = {
	0x09,								//bLength: length of this descriptor in bytes (9)
	USB_DESC_CONFIGURATION,				//bDescriptorType: configuration descriptor
	I16_TO_LE_BA(34),					//wTotalLen: Total length, including attached interface and endpoint descriptors
	0x01,								//bNumInterfaces: Number of interfaces (1)
	0x01,								//bConfigurationValue: Number to set to activate this config
	0x00,								//iConfiguration: configuration string index (0 = not available)
	0x80,								//bmAttributes: Not self-powered, no remote wakeup
	0x32,								//bMaxPower: Max power in 2mA steps (0x32 = 50 = 100mA)
	
	// Interface 0: HID
	0x09,								//bLength: length of this descriptor in bytes (9)
	USB_DESC_INTERFACE,					//bDescriptorType: configuration descriptor
	0x00,								//bInterfaceNumber: Interface index, 0-based
	0x00,								//bAlternateSetting
	0x01,								//bNumEndpoints: Number of endpoints excluding control endpoint
	USB_CLASS_HID,						//bInterfaceClass: interface class
	0x01,								//bInterfaceSubClass 1 = boot interface (0 for non-boot interface)
	0x02,								//bInterfaceProtocol 2 = boot mouse (1 for boot keyboard, 0 for all others)
	0x00,								//iInterface: String index (0x00 = not available)

	// HID descriptor (inserted between interface and endpoints for HID devices)
	0x09,								//bLength: length of HID descriptor in bytes
	USB_DESC_HID_HID,					//bDescriptorType: HID descriptor
	I16_TO_LE_BA(0x0100),				//bcdHID: version 1.0
	0x09,								//bCountryCode: 9 = German (HID spec 1.11, section 6.2.1 - 0=not supported, 33=US)
	0x01,								//bNumDescriptors: We have one descriptor
	USB_DESC_HID_REPORT,				//bDescriptorType: It is a report descriptor
	I16_TO_LE_BA(sizeof(mouse_reportDescriptor)),	//wDescriptorLength: length of report descriptor
	
	// Endpoint descriptor: 1 interrupt in
	0x07,							    //bLength: Length of endpoint descriptor in bytes
	USB_DESC_ENDPOINT,					//bDescriptorType: This is an endpoint descriptor
	0x81,								//bEndpointAddress: Logical endpoint 1, direction IN
	USB_EPTYPE_INTERRUPT,				//bmAttributesL: Interrput endpoint
	I16_TO_LE_BA(IN_REPORT_LENGTH),		//wMaxPacketSize: IN report size
	0x0a								//bInterval: Poll interval in ms
};


const uint8_t mouse_languages[] = {
	0x04,								//bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	I16_TO_LE_BA(0x0409)				//wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};
	
const uint8_t mouse_manufacturerName[] = {
	0x22,								//bLength: length of this descriptor in bytes (34)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'P',0,'r',0,'e',0,'s',0,'s',0,' ',0,'A',0,'n',0,'y',0,' ',0,'K',0,'e',0,'y',0,' ',0,'U',0,'G',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t mouse_deviceName[] = {
	0x12,								//bLength: length of this descriptor in bytes (18)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,'0',0,'x',0			//bString[]: String (UTF16LE, not terminated)
};

const uint8_t mouse_serialName[] = {
	0x0a,								//bLength: length of this descriptor in bytes (10)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'V',0,'1',0,'.',0,'0',0				//bString[]: String (UTF16LE, not terminated)
};

const USBHID_Behaviour_Struct mouse_behaviour = {
	MAKE_USBHID_BASE_BEHAVIOUR,
	0,
	mouse_configDescriptor+18,
	mouse_reportDescriptor,
	sizeof(mouse_reportDescriptor),
	inReportHandler,
	outReportHandler,
	inBuffer,
	outBuffer,
	&idleValue,
	&currentProtocol
};

const USB_Device_Definition mouse_deviceDefinition = {
        mouse_deviceDescriptor,
        1,
        { mouse_configDescriptor },
        4,
        { mouse_languages, mouse_manufacturerName, mouse_deviceName, mouse_serialName },
        1,
        { (USB_Behaviour_Struct*)(&mouse_behaviour) }
};


/** MAIN PROGRAM FROM HERE */

void main () {
	anypio_digital_input_set(BUTTON, PULL_UP);
	anypio_analog_input_set(X, true);
	anypio_analog_input_set(Y, true);
	
	USB_Init(&mouse_deviceDefinition, &mouse_device);

	USB_SoftConnect (&mouse_device);

	SYSCON_StartSystick_10ms();
}

void systick () {
	x = anypio_analog_read(X);
	y = 1024 - anypio_analog_read(Y);
	button = !anypio_read(BUTTON);
	if (avgCounter < AVG_COUNT) {	//Still in averaging phase
		avgX += x;
		avgY += y;
		avgCounter++;
		if (avgCounter == AVG_COUNT) {
			avgX = (avgX / AVG_COUNT) - (1 << (SENS_SHIFT-1));
			avgY = (avgY / AVG_COUNT) - (1 << (SENS_SHIFT-1));
		}
	} else {						//Send data
 		USBHID_PushReport (&mouse_device, &mouse_behaviour, USB_HID_REPORTTYPE_INPUT, 0);
 	}
}

