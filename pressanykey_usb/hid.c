#include "hid.h"
#include "usbhidspec.h"


/** this string descriptor is always returned as string 0 - it tells the host
 * about available string languages */

const uint8_t languagesString[] = {
  //bLength: length of this descriptor in bytes (4)
	0x04,		
  //bDescriptorType: string descriptor
	USB_DESC_STRING,	
  //wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
	0x09,0x04					
};

/** called when the data from a set report command has arrived */
void hidSetReportDataComplete(USB_Device_Struct* device) {
	HID_Device_Struct* hid = (HID_Device_Struct*)(device->refcon);
	if (hid->outReportHandler) {
		hid->outReportHandler(hid,
							  device->currentCommand.wValueH,
							  device->currentCommand.wValueL,
							  (device->currentCommand.wLengthH << 8) |
							  device->currentCommand.wLengthL);
	}
}

/** handler for HID-specific USB commands */
bool hidCommandHandler(USB_Device_Struct* device) {
	HID_Device_Struct* hid = (HID_Device_Struct*)(device->refcon);
	uint16_t maxTransferLen = (device->currentCommand.wLengthH << 8) | device->currentCommand.wLengthL;
	switch (device->currentCommand.bRequest) {
    //we don't handle this command, but we use it as a trigger to init our state
		case USB_REQ_SET_ADDRESS: 				
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) break;
			if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_DEVICE) break;
			hid->currentProtocol = 1;
			hid->idleValue = 128;
			break;
    //We handle HID-specific reports here.
		case USB_REQ_GET_DESCRIPTOR:		
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) break;
			if (device->currentCommand.wIndexL == hid->interfaceNumber) {
				switch (device->currentCommand.wValueH) {
					case USB_DESC_HID_HID:
						device->currentCommandDataBase = hid->hidDescriptor;
						device->currentCommandDataRemaining = hid->hidDescriptor[0];
						if (device->currentCommandDataRemaining > maxTransferLen) {
							device->currentCommandDataRemaining = maxTransferLen;
						}
						return true;
					case USB_DESC_HID_REPORT:
						device->currentCommandDataBase = hid->reportDescriptor;
						device->currentCommandDataRemaining = hid->reportDescriptorLen;
						if (device->currentCommandDataRemaining > maxTransferLen) {
							device->currentCommandDataRemaining = maxTransferLen;
						}
						return true;
				}
			}
			break;
		case USB_REQ_HID_GETIDLE:
			if (device->currentCommand.bmRequestType != (USB_RT_DIR_DEVICE_TO_HOST | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (device->currentCommand.wIndexL != hid->interfaceNumber) break;
			device->currentCommandDataBase = &(hid->idleValue);
			device->currentCommandDataRemaining = 1;
			return true;
			break;
		case USB_REQ_HID_SETIDLE:
			if (device->currentCommand.bmRequestType != (USB_RT_DIR_HOST_TO_DEVICE | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (device->currentCommand.wIndexL != hid->interfaceNumber) break;
			hid->idleValue = device->currentCommand.wValueH;
			return true;
			break;
		case USB_REQ_HID_GETPROTOCOL:
			if (device->currentCommand.bmRequestType != (USB_RT_DIR_DEVICE_TO_HOST | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (device->currentCommand.wIndexL != hid->interfaceNumber) break;
			device->currentCommandDataBase = &(hid->currentProtocol);
			device->currentCommandDataRemaining = 1;
			return true;
			break;
		case USB_REQ_HID_SETPROTOCOL:
			if (device->currentCommand.bmRequestType != (USB_RT_DIR_HOST_TO_DEVICE | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (device->currentCommand.wIndexL != hid->interfaceNumber) break;
			hid->currentProtocol = device->currentCommand.wValueL;
			return true;
			break;
		case USB_REQ_HID_GETREPORT:
			if (device->currentCommand.bmRequestType != (USB_RT_DIR_DEVICE_TO_HOST | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (device->currentCommand.wIndexL != hid->interfaceNumber) break;
			if ((!hid->inReportHandler) || (!hid->inBuffer)) break;
			device->currentCommandDataBase = hid->inBuffer;
			device->currentCommandDataRemaining = (hid->inReportHandler)(hid,
																			device->currentCommand.wValueH,
																			device->currentCommand.wValueL);   
			return true;
			break;
		case USB_REQ_HID_SETREPORT:
			if (device->currentCommand.bmRequestType != (USB_RT_DIR_HOST_TO_DEVICE | USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE)) break;
			if (device->currentCommand.wIndexL != hid->interfaceNumber) break;
			if ((!hid->outReportHandler) || (!hid->outBuffer)) break;
			device->currentCommandDataBase = hid->outBuffer;
			device->currentCommandDataRemaining = (device->currentCommand.wLengthH << 8) |
														device->currentCommand.wLengthL;
      //Call us again when the data is there
			device->controlOutDataCompleteCallback = hidSetReportDataComplete;	
			return true;
			break;
	}
	return false;
}

void hidEndpointDataHandler(USB_Device_Struct* device, uint8_t epIdx) {
	//Do nothing, just add a handler to prevent pipe from stalling (will stall if no handler is there)
}

void HIDInit(	USB_Device_Struct* usbDevice,
				HID_Device_Struct* hidDevice,
				int interfaceNum,
				const uint8_t* deviceDesc,
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
	usbDevice->deviceDescriptor = deviceDesc;
	usbDevice->configurationCount = 1;
	usbDevice->configurationDescriptors[0] = configDesc;
	usbDevice->stringCount = 4;
	usbDevice->strings[0] = languagesString;
	usbDevice->strings[1] = string1;
	usbDevice->strings[2] = string2;
	usbDevice->strings[3] = string3;
	usbDevice->extendedControlSetupCallback = hidCommandHandler;
	usbDevice->endpointDataCallback = hidEndpointDataHandler;
	usbDevice->frameCallback = NULL;
	usbDevice->refcon = hidDevice;
	//we don't change values inside
	hidDevice->hidDescriptor = (uint8_t*)hidDesc;		
	//we don't change values inside
	hidDevice->reportDescriptor = (uint8_t*)reportDesc;	
	hidDevice->reportDescriptorLen = reportDescLen;
	hidDevice->inBuffer = inReportBuffer;
	hidDevice->outBuffer = outReportBuffer;
	hidDevice->inReportHandler = inReportHandler;
	hidDevice->outReportHandler = outReportHandler;
	hidDevice->interfaceNumber = interfaceNum;
	USB_Init(usbDevice);
}

void HIDPushReport(HID_Device_Struct* hid, USB_HID_REPORTTYPE reportType, uint8_t reportId) {
	if ((!hid->inReportHandler) || (!hid->inBuffer)) return;
	uint16_t len = (hid->inReportHandler)(hid, reportType, reportId);
	USB_EP_Write(hid->usbDevice, 3, hid->inBuffer, len);
}

