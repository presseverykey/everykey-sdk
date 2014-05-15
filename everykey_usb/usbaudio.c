/** Everykey USB stack - USB Audio behaviour implementation */

#include "usbaudio.h"



/** OUT data transferred */
bool USBAudio_ExtendedControlSetupHandler2(USB_Device_Struct* device) {
	if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) return false;
	const USBAudio_Behaviour_Struct* audio = (const USBAudio_Behaviour_Struct*)(device->callbackRefcon);
	switch (device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
		case USB_RT_RECIPIENT_INTERFACE:
			//must address our control interface
			if (device->currentCommand.wIndexL != audio->controlInterface) return false;
			if (audio->setControlValueCallback) {
				return (audio->setControlValueCallback)(device, audio,
														device->currentCommand.bRequest,
														device->currentCommand.wIndexH,
														device->currentCommand.wValueL,
														device->currentCommand.wValueH,
														device->commandDataBuffer,
														(device->currentCommand.wLengthH << 8) |
														device->currentCommand.wLengthL);
			}
			break;
		case USB_RT_RECIPIENT_ENDPOINT:
			//TODO: Check endpoints ***
			if (audio->setEndpointValueCallback) {
			return (audio->setEndpointValueCallback)(device, audio,
													 device->currentCommand.bRequest,
													 device->currentCommand.wIndexL,
													 device->currentCommand.wValueL,
													 device->commandDataBuffer,
													 (device->currentCommand.wLengthH << 8) |
													 device->currentCommand.wLengthL);
			}
			break;
	}
	return false;
}

bool USBAudio_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {

	if ((device->currentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_CLASS) return false;
	
	const USBAudio_Behaviour_Struct* audio = (const USBAudio_Behaviour_Struct*)behaviour;

	bool result = false;
	uint16_t len = (device->currentCommand.wLengthH << 8) | device->currentCommand.wLengthL;

	//set requests will be handled after the data phase, but we check now if the request may be for us
	if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) == USB_RT_DIR_HOST_TO_DEVICE) {
		
		if ((len < 1) || (len > USB_MAX_COMMAND_DATA_SIZE)) return false;
		if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) == USB_RT_RECIPIENT_INTERFACE) {
			if (device->currentCommand.wIndexL != audio->controlInterface) return false; //not our interface
			if (!(audio->setControlValueCallback)) return false;	//set control value not supported
		} else if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) == USB_RT_RECIPIENT_ENDPOINT) {
			if ((device->currentCommand.wIndexL != audio->inStreamEndpoint) &&
				(device->currentCommand.wIndexL != audio->outStreamEndpoint)) return false;	//not our endpoint
			if (!(audio->setEndpointValueCallback)) return false;	//set endpoint value not supported
		} else return false;	//Not our recipient

		//if we reached this point, the request is for us. Read data and set callback to handle it.
		device->currentCommandDataBase = device->commandDataBuffer;
		device->currentCommandDataRemaining = len;
		device->callbackRefcon = (void*)behaviour;
		device->controlOutDataCompleteCallback = USBAudio_ExtendedControlSetupHandler2;
		result = true;
	} else {
		switch (device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
			case USB_RT_RECIPIENT_INTERFACE:
				if (device->currentCommand.wIndexL != audio->controlInterface) break;
				if (!(audio->getControlValueCallback)) break;
				result = audio->getControlValueCallback(device, audio,
														device->currentCommand.bRequest,
														device->currentCommand.wIndexH,
														device->currentCommand.wValueL,
														device->currentCommand.wValueH,
														device->commandDataBuffer,
														len);
				break;
			case USB_RT_RECIPIENT_ENDPOINT:
				if ((device->currentCommand.wIndexL != audio->inStreamEndpoint) &&
					(device->currentCommand.wIndexL != audio->outStreamEndpoint)) break;
				if (!(audio->getEndpointValueCallback)) break;
				result = audio->getEndpointValueCallback(device, audio,
														 device->currentCommand.bRequest,
														 device->currentCommand.wIndexL,
														 device->currentCommand.wValueL,
														 device->commandDataBuffer,
														 len);
				break;
		}
		if (result) {
			device->currentCommandDataBase = device->commandDataBuffer;
			device->currentCommandDataRemaining = len;
		}	
	}
	return result;
}

void USBAudio_FrameHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	const USBAudio_Behaviour_Struct* audio = (const USBAudio_Behaviour_Struct*)behaviour;
	if (audio->frameCallback) (audio->frameCallback)(device,audio);
}

bool USBAudio_InterfaceAltHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t interface, uint8_t newAlt) {
	const USBAudio_Behaviour_Struct* audio = (const USBAudio_Behaviour_Struct*)behaviour;
	if ((interface != audio->inStreamInterface) &&
		(interface != audio->outStreamInterface) && 
		(interface != audio->controlInterface))return false;
	if (((USBAudio_Behaviour_Struct*)behaviour)->altChangeCallback) {
		return (((USBAudio_Behaviour_Struct*)behaviour)->altChangeCallback)(device,
																			(USBAudio_Behaviour_Struct*)behaviour,
																			interface,
																			newAlt);
	} else return true;
}

void USBAudio_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	const USBAudio_Behaviour_Struct* audio = (const USBAudio_Behaviour_Struct*)behaviour;
	if (audio->configChangeCallback) (audio->configChangeCallback)(device,audio);
}

