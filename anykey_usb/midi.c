#include "midi.h"

/** called when the data from a set command has arrived */
bool USBMIDI_SetReportDataComplete(USB_Device_Struct* device) {

	return false;	//TODO
}

/** handler for MIDI-specific USB commands */
bool USBMIDI_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {

	// we only handle requests to our interface	
	if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_INTERFACE) return false;

	const USBMIDI_Behaviour_Struct* midi = (const USBMIDI_Behaviour_Struct*)behaviour;
	
	if (device->currentCommand.wIndexL != midi->interfaceNumber) return false;

	uint16_t maxTransferLen = (device->currentCommand.wLengthH << 8) | device->currentCommand.wLengthL;

	//MIDI does not modify standard requests, so we can skip to class-specific requests
	
	//From now, only handle class-specific requests
	if ((device->currentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_CLASS) return false;
	
	switch (device->currentCommand.bRequest) {
		//TODO ********
		return false;
	}
	return false;
}

bool USBMIDI_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx) {
	const USBMIDI_Behaviour_Struct* midi = (const USBMIDI_Behaviour_Struct*)behaviour;
	if ((midi->inBuffer) && (epIdx == midi->inDataEndpoint)) {
		//TODO **********************
		return true;
	}
	if ((midi->outBuffer) && (epIdx == midi->outDataEndpoint)) {
		uint16_t transfer = USB_EP_Read(device, midi->outDataEndpoint, midi->outBuffer, USB_MAX_BULK_DATA_SIZE);
		if ((transfer>0) && (midi->outDataHandler)) midi->outDataHandler(device, midi, transfer);
		return true;
	}
	return false;
}

/** we use this callback to reset our protocol and idle values */
void USBMIDI_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	const USBMIDI_Behaviour_Struct* hid = (const USBMIDI_Behaviour_Struct*)behaviour;
	//TODO: Setup ************
}

