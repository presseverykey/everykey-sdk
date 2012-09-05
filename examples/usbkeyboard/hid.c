#include "hid.h"

const uint8_t hidInterfaceNumber = 0;
uint8_t hidCurrentProtocol;
uint8_t* hidHidDescriptor;
uint8_t* hidReportDescriptor;
uint16_t hidReportDescriptorLen;
uint8_t* hidInBuffer;
uint8_t* hidOutBuffer;
HidInReportHandler hidInReportHandler;
HidOutReportHandler hidOutReportHandler;
uint8_t hidIdleValue;

USB_Device_Struct hidUSBDevice;

/** this string descriptor is always returned as string 0 - it tells the host about available string languages */
const uint8_t languagesString[] = {
	0x04,							//bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	0x09,0x04						//wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};

/** called when the data from a set report command has arrived */
void hidSetReportDataComplete() {
	if (hidOutReportHandler) {
		hidOutReportHandler(	usbCurrentCommand.wValueH, usbCurrentCommand.wValueL,
								(usbCurrentCommand.wLengthH << 8) | usbCurrentCommand.wLengthL);
	}
}

/** handler for HID-specific USB commands */
bool hidCommandHandler() {
	uint16_t maxTransferLen = (usbCurrentCommand.wLengthH << 8) | usbCurrentCommand.wLengthL;
	switch (usbCurrentCommand.bRequest) {
		case USB_REQ_SET_ADDRESS: 				//we don't handle this command, but we use it as a trigger to init our globals
			if ((usbCurrentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) break;
			if ((usbCurrentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_DEVICE) break;
			hidCurrentProtocol = 1;
			hidIdleValue = 128;
			break;
		case USB_REQ_GET_DESCRIPTOR:			//We handle HID-specific reports here.
			if ((usbCurrentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) break;
			if (usbCurrentCommand.wIndexL == hidInterfaceNumber) {
				switch (usbCurrentCommand.wValueH) {
					case USB_DESC_HID_HID:
						usbCurrentCommandDataBase = hidHidDescriptor;
						usbCurrentCommandDataRemaining = usbCurrentCommandDataBase[0];
						if (usbCurrentCommandDataRemaining > maxTransferLen) usbCurrentCommandDataRemaining = maxTransferLen;
						return true;
					case USB_DESC_HID_REPORT:
						usbCurrentCommandDataBase = hidReportDescriptor;
						usbCurrentCommandDataRemaining = hidReportDescriptorLen;
						if (usbCurrentCommandDataRemaining > maxTransferLen) usbCurrentCommandDataRemaining = maxTransferLen;
						return true;
				}
			}
			break;
		case USB_REQ_HID_GETIDLE:
			if (usbCurrentCommand.bmRequestType != (USB_RT_DIR_DEVICE_TO_HOST | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (usbCurrentCommand.wIndexL != hidInterfaceNumber) break;
			usbCurrentCommandDataBase = &hidIdleValue;
			usbCurrentCommandDataRemaining = 1;
			return true;
			break;
		case USB_REQ_HID_SETIDLE:
			if (usbCurrentCommand.bmRequestType != (USB_RT_DIR_HOST_TO_DEVICE | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (usbCurrentCommand.wIndexL != hidInterfaceNumber) break;
			hidIdleValue = usbCurrentCommand.wValueH;
			return true;
			break;
		case USB_REQ_HID_GETPROTOCOL:
			if (usbCurrentCommand.bmRequestType != (USB_RT_DIR_DEVICE_TO_HOST | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (usbCurrentCommand.wIndexL != hidInterfaceNumber) break;
			usbCurrentCommandDataBase = &hidCurrentProtocol;
			usbCurrentCommandDataRemaining = 1;
			return true;
			break;
		case USB_REQ_HID_SETPROTOCOL:
			if (usbCurrentCommand.bmRequestType != (USB_RT_DIR_HOST_TO_DEVICE | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (usbCurrentCommand.wIndexL != hidInterfaceNumber) break;
			hidCurrentProtocol = usbCurrentCommand.wValueL;
			return true;
			break;
		case USB_REQ_HID_GETREPORT:
			if (usbCurrentCommand.bmRequestType != (USB_RT_DIR_DEVICE_TO_HOST | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (usbCurrentCommand.wIndexL != hidInterfaceNumber) break;
			if ((!hidInReportHandler) || (!hidInBuffer)) break;
			usbCurrentCommandDataBase = hidInBuffer;
			usbCurrentCommandDataRemaining = (*hidInReportHandler)(usbCurrentCommand.wValueH, usbCurrentCommand.wValueL);
			return true;
			break;
		case USB_REQ_HID_SETREPORT:
			if (usbCurrentCommand.bmRequestType != (USB_RT_DIR_HOST_TO_DEVICE | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (usbCurrentCommand.wIndexL != hidInterfaceNumber) break;
			if ((!hidOutReportHandler) || (!hidOutBuffer)) break;
			usbCurrentCommandDataBase = hidOutBuffer;
			usbCurrentCommandDataRemaining = (usbCurrentCommand.wLengthH << 8) | usbCurrentCommand.wLengthL;
			usbControlOutDataCompleteCallback = hidSetReportDataComplete;	//Call us again when the data is there
			return true;
			break;
	}
	return false;
}

void hidEndpointDataHandler(uint8_t epIdx) {
	//Do nothing, just add a handler to prevent pipe from stalling (will stall if no handler is there)
}


USB_Device_Struct myUSBDevice;


void HIDInit(	const uint8_t* deviceDesc,
				const uint8_t* configDesc,
				const uint8_t* string1,
				const uint8_t* string2,
				const uint8_t* string3,
				const uint8_t* hidDesc,
				const uint8_t* reportDesc,
				uint16_t reportDescLen,
				uint8_t* inReportBuffer,
				uint8_t* outReportBuffer,
				HidInReportHandler inReportHandler,
				HidOutReportHandler outReportHandler) {
	hidUSBDevice.deviceDescriptor = deviceDesc;
	hidUSBDevice.configurationCount = 1;
	hidUSBDevice.configurationDescriptors[0] = configDesc;
	hidUSBDevice.stringCount = 4;
	hidUSBDevice.strings[0] = languagesString;
	hidUSBDevice.strings[1] = string1;
	hidUSBDevice.strings[2] = string2;
	hidUSBDevice.strings[3] = string3;
	hidUSBDevice.extendedControlSetupCallback = hidCommandHandler;
	hidUSBDevice.endpointDataCallback = hidEndpointDataHandler;
	hidHidDescriptor = (uint8_t*)hidDesc;		//we don't change values inside
	hidReportDescriptor = (uint8_t*)reportDesc;	//we don't change values inside
	hidReportDescriptorLen = reportDescLen;
	hidInBuffer = inReportBuffer;
	hidOutBuffer = outReportBuffer;
	hidInReportHandler = inReportHandler;
	hidOutReportHandler = outReportHandler;

	USB_Init(&hidUSBDevice);
}

void HIDPushReport(HID_REPORTTYPE reportType, uint8_t reportId) {
	if ((!hidInReportHandler) || (!hidInBuffer)) return;
	uint16_t len = hidInReportHandler(reportType, reportId);
	USB_EP_Write(3, hidInBuffer, len);
}

