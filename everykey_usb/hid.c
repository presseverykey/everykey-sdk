#include "hid.h"
#include "usbhidspec.h"

/** called when the data from a set report command has arrived */
bool USBHID_SetReportDataComplete(USB_Device_Struct* device) {
	const USBHID_Behaviour_Struct* hid = (const USBHID_Behaviour_Struct*)(device->callbackRefcon);
	if (hid->outReportHandler) {
		(hid->outReportHandler)(device,
								hid,
								device->currentCommand.wValueH,
								device->currentCommand.wValueL,
								(device->currentCommand.wLengthH << 8) |
								device->currentCommand.wLengthL);
		return true;
	} else {
		return false;
	}
}

/** handler for HID-specific USB commands */
bool USBHID_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	// we only handle requests to our interface	
	if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_INTERFACE) {
		return false;
	}

	const USBHID_Behaviour_Struct* hid = (const USBHID_Behaviour_Struct*)behaviour;
	
	if (device->currentCommand.wIndexL != hid->interfaceNumber) {
		return false;
	}

	uint16_t maxTransferLen = (device->currentCommand.wLengthH << 8) | device->currentCommand.wLengthL;

	//check extensions of standard requests (i.e. get interface)
	if ((device->currentCommand.bmRequestType == USB_RT_DIR_DEVICE_TO_HOST | USB_RT_TYPE_STANDARD | USB_RT_RECIPIENT_INTERFACE) &&
		(device->currentCommand.bRequest == USB_REQ_GET_DESCRIPTOR)) {
		if (device->currentCommand.wIndexL == hid->interfaceNumber) {
			switch (device->currentCommand.wValueH) {
				case USB_DESC_HID_HID:
					
					//we remove const, but we promise to only read it
					device->currentCommandDataBase = (uint8_t*)(hid->hidDescriptor);	
					device->currentCommandDataRemaining = hid->hidDescriptor[0];
					if (device->currentCommandDataRemaining > maxTransferLen) {
						device->currentCommandDataRemaining = maxTransferLen;
					}
					return true;
				case USB_DESC_HID_REPORT:
					//we remove const, but we promise to only read it
					device->currentCommandDataBase = (uint8_t*)(hid->reportDescriptor);
					device->currentCommandDataRemaining = hid->reportDescriptorLen;
					if (device->currentCommandDataRemaining > maxTransferLen) {
						device->currentCommandDataRemaining = maxTransferLen;
					}
					return true;
			}
		}
	}
	
	//From now, only handle class-specific requests
	if ((device->currentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_CLASS) return false;
	
	switch (device->currentCommand.bRequest) {
		case USB_REQ_GET_DESCRIPTOR:		
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) break;
			if (device->currentCommand.wIndexL == hid->interfaceNumber) {
			}
			break;
		case USB_REQ_HID_GETIDLE:
		if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) break;
			device->currentCommandDataBase = hid->idleValue;
			device->currentCommandDataRemaining = 1;
			return true;
			break;
		case USB_REQ_HID_SETIDLE:
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) break;
			*(hid->idleValue) = device->currentCommand.wValueH;
			return true;
			break;
		case USB_REQ_HID_GETPROTOCOL:
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) break;
			device->currentCommandDataBase = hid->currentProtocol;
			device->currentCommandDataRemaining = 1;
			return true;
			break;
		case USB_REQ_HID_SETPROTOCOL:
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) break;
			*(hid->currentProtocol) = device->currentCommand.wValueL;
			return true;
			break;
		case USB_REQ_HID_GETREPORT:
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) break;
			if ((hid->inReportHandler) && (hid->inBuffer)) {
				device->currentCommandDataBase = hid->inBuffer;
				device->currentCommandDataRemaining = (hid->inReportHandler)(device,hid,
																			device->currentCommand.wValueH,
																			device->currentCommand.wValueL);  
				return true;
			}
			break;
		case USB_REQ_HID_SETREPORT:
			if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) break;
			if ((hid->outReportHandler) && (hid->outBuffer)) {
				device->currentCommandDataBase = hid->outBuffer;
				device->currentCommandDataRemaining = (device->currentCommand.wLengthH << 8) |
														device->currentCommand.wLengthL;
				//Call us again when the data is there
				device->callbackRefcon = (void*)hid;
				device->controlOutDataCompleteCallback = USBHID_SetReportDataComplete;	
				return true;
			}
			break;
	}
	return false;
}

bool USBHID_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx) {
	//TODO: Check against our endpoint ************
	//Right now, we do nothing, just accept and ignore everything
}

/** we use this callback to reset our protocol and idle values */
void USBHID_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	const USBHID_Behaviour_Struct* hid = (const USBHID_Behaviour_Struct*)behaviour;
	*(hid->currentProtocol) = 1;
	*(hid->idleValue) = 128;
}

void USBHID_PushReport(USB_Device_Struct* device,
					   const USBHID_Behaviour_Struct* hid,
					   USB_HID_REPORTTYPE reportType,
					   uint8_t reportId) {
	if ((hid->inReportHandler) && (hid->inBuffer)) {
		uint16_t len = (hid->inReportHandler)(device, hid, reportType, reportId);
		USB_EP_Write(device, 3, hid->inBuffer, len);
	}
}

