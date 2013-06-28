#include "cdc.h"
#include "../anykey/endian.h"

/** this function is called when the data phase of a SetCommFeature command completed. Read data is in the commandDataBuffer. */
bool USBCDC_SetCommFeatureCompleted(USB_Device_Struct* device) {
	USBCDC_Behaviour_Struct* cdc = (USBCDC_Behaviour_Struct*)device->callbackRefcon;
	switch (device->currentCommand.wValueL) {
		case USB_CDC_COMM_FEATURE_ABSTRACT_STATE:
			*(cdc->idle) = device->commandDataBuffer[0] & USB_CDC_COMM_FEATURE_ABSTRACT_STATE_IDLE;
			return true;
			break;
/*	Country code selector not supported
	case USB_CDC_COMM_FEATURE_COUNTRY_SETTING:
			return true;
			break;
*/		default:
			return false;
			break;
	}
}

/** this function is called when the data phase of a SetLineCoding command completed. Read data is in the commandDataBuffer. */
bool USBCDC_SetLineCodingCompleted(USB_Device_Struct* device) {
	USBCDC_Behaviour_Struct* cdc = (USBCDC_Behaviour_Struct*)device->callbackRefcon;
	USB_CDC_Linecoding_Struct* newLineCoding = (USB_CDC_Linecoding_Struct*)(device->commandDataBuffer);
	newLineCoding->dwDTERRate = U32_TO_LE(newLineCoding->dwDTERRate);
	
	bool ok = (cdc->lineCodingChangeCallback) ? cdc->lineCodingChangeCallback(device,cdc,newLineCoding) : true;
	if (ok) {
		cdc->currentLineCoding->dwDTERRate = newLineCoding->dwDTERRate;
		cdc->currentLineCoding->bCharFormat = newLineCoding->bCharFormat;
		cdc->currentLineCoding->bParityType = newLineCoding->bParityType;
		cdc->currentLineCoding->bDataBits = newLineCoding->bDataBits;
	}
	return ok;
}

bool USBCDC_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	
	bool handled = false;
	USBCDC_Behaviour_Struct* cdc = (USBCDC_Behaviour_Struct*)behaviour;
	USB_Setup_Packet* req = &(device->currentCommand);
	
	if (req->wIndexL != cdc->controlInterface) return;	//must be targeted at our interface
	uint16_t dataLen = (req->wLengthH << 8) | req->wLengthL;
	
	if (req->bmRequestType == (USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE | USB_RT_DIR_HOST_TO_DEVICE)) {
		
		//class specific interface write  request
		
		switch (req->bRequest) {
			case USB_CDC_REQUEST_SET_COMM_FEATURE:
				if ((dataLen == 2) && (req->wValueL == USB_CDC_COMM_FEATURE_ABSTRACT_STATE) || (req->wValueL == USB_CDC_COMM_FEATURE_ABSTRACT_STATE)) {
					device->currentCommandDataBase = device->commandDataBuffer;
					device->currentCommandDataRemaining = dataLen;
					device->controlOutDataCompleteCallback = USBCDC_SetCommFeatureCompleted;
					device->callbackRefcon = (void*)cdc;
					handled = true;
				}
				break;
			case USB_CDC_REQUEST_CLEAR_COMM_FEATURE:
				switch (req->wValueL) {
					case USB_CDC_COMM_FEATURE_ABSTRACT_STATE:
						*(cdc->idle) = false;
						if (cdc->idleChangeCallback) cdc->idleChangeCallback(device, cdc);
						handled = true;
						break;
						/* Country code not supported 
						 case USB_CDC_COMM_FEATURE_COUNTRY_SETTING:
						 handled = true;
						 break;
						 */
					default:
						break;
				}
				break;
			case USB_CDC_REQUEST_SET_LINE_CODING:
				if (dataLen == sizeof(USB_CDC_Linecoding_Struct)) {
					device->currentCommandDataBase = device->commandDataBuffer;
					device->currentCommandDataRemaining = dataLen;
					device->controlOutDataCompleteCallback = USBCDC_SetLineCodingCompleted;
					device->callbackRefcon = (void*)cdc;
					handled = true;
				}
				break;
			case USB_CDC_REQUEST_SEND_BREAK:
				if (cdc->breakCallback) cdc->breakCallback(device, cdc, (req->wValueH << 8) | req->wValueL);
				handled = true;
				break;
			case USB_CDC_REQUEST_SET_CONTROL_LINE_STATE:
				*(cdc->controlLineState) = req->wValueL;
				if (cdc->controlLineChangeCallback) cdc->controlLineChangeCallback(device, cdc);
				handled = true;
				break;
			default:
				break;
				
		}
	} else if (req->bmRequestType == (USB_RT_TYPE_CLASS | USB_RT_RECIPIENT_INTERFACE | USB_RT_DIR_DEVICE_TO_HOST)) {
		
		//reads (Get...) for the control interface
		switch (req->bRequest) {
			case USB_CDC_REQUEST_GET_COMM_FEATURE:	//Host wants comm feature bit mask
				switch (req->wValueL) {
					case USB_CDC_COMM_FEATURE_ABSTRACT_STATE:
						if (dataLen == 2) {
							device->commandDataBuffer[0] = *(cdc->idle) ? USB_CDC_COMM_FEATURE_ABSTRACT_STATE_IDLE : 0;
							device->commandDataBuffer[1] = 0;
							device->currentCommandDataBase = device->commandDataBuffer;
							device->currentCommandDataRemaining = dataLen;
							handled = true;
						}
						break;
						/* Country code not supported
						 case USB_CDC_COMM_FEATURE_COUNTRY_SETTING: //host wants country code
						 if (len == 2) {
						 ((uint16_t*)(device->commandDataBuffer))[0] = U16_TO_LE(cdc->countryCode);
						 device->currentRequestDataBase = device->commandDataBuffer;
						 device->currentCommandDataRemaining = len;
						 handled = true;
						 }
						 break;
						 */
					default:
						break;
				}
			case USB_CDC_REQUEST_GET_LINE_CODING:			//host wants line coding
				if (dataLen == sizeof(USB_CDC_Linecoding_Struct)) {
					device->currentCommandDataBase = (uint8_t*)(cdc->currentLineCoding);
					device->currentCommandDataRemaining = dataLen;
					handled = true;
				}
				break;
				/*					
				 case USB_CDC_REQUEST_SET_CONTROL_LINE_STATE:
				 //WORKAROUND FOR OSX BUG! This request should be issued as host-to-device, but it is not
				 *(cdc->controlLineState) = req->wValueL;
				 if (cdc->controlLineChangeCallback) cdc->controlLineChangeCallback(device, cdc);
				 handled = true;
				 break;
				 */					
			default:
				break;
		}
	}
	
	return handled;
}

bool USBCDC_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx) {
	USBCDC_Behaviour_Struct* cdc = (USBCDC_Behaviour_Struct*)behaviour;
	bool handled = false;
	uint8_t tmpBuffer[USB_MAX_BULK_DATA_SIZE];
	
	if (epIdx == cdc->interruptEndpoint) {
		//Interrupt buffer might be free again: accept, don't care
		handled = true;
	} else if (epIdx == cdc->dataInEndpoint) {
		//Device-To-Host endpoint might be free again: check rebuffering
		bool canWrite = (!USB_EP_GetFull(device, cdc->dataInEndpoint)) && (!USB_EP_GetStall(device, cdc->dataInEndpoint));
		if (canWrite) {
			uint16_t transfer = RingBufferReadBuffer(&(cdc->deviceToHostBuffer), tmpBuffer, USB_MAX_BULK_DATA_SIZE);
			transfer = USB_EP_Write(device, cdc->dataInEndpoint, tmpBuffer, transfer);
		}
		handled = true;
	} else if (epIdx == cdc->dataOutEndpoint) {
		//HostToDevice endpoint might have data: check rebuffering
		bool canRead = USB_EP_GetFull(device, cdc->dataOutEndpoint) && (!USB_EP_GetStall(device, cdc->dataOutEndpoint));
		if (canRead) {
			uint16_t available = RingBufferWriteBytesAvailable(&(cdc->deviceToHostBuffer));
/* TODO: Right now, we only accept data from the host if the ring buffer can accept a full bulk block
Instead, we should query how many bytes are actually available in the endpoint buffer and check if
the ring buffer can hold that data. The current solution is overflow-safe,
but might not yield best buffering performance. */
			if (available >= USB_MAX_BULK_DATA_SIZE) {	
				uint16_t transfer = USB_EP_Read(device, cdc->dataOutEndpoint, tmpBuffer, USB_MAX_BULK_DATA_SIZE);
				transfer = RingBufferWriteBuffer(&(cdc->hostToDeviceBuffer), tmpBuffer, transfer);
				if ((transfer>0) && (cdc->dataAvailableCallback)) cdc->dataAvailableCallback(device, cdc);
			}
		}
		handled = true;
	}
	return handled;
}


void USBCDC_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	//we use a config change to reset
	USBCDC_ResetBehaviour((USBCDC_Behaviour_Struct*)behaviour);
}

// Client-initiated activities

void USBCDC_ResetBehaviour(const USBCDC_Behaviour_Struct* cdc) {
	*(cdc->controlLineState) = 0;
	*(cdc->idle) = false;
	RingBufferInit(&(cdc->hostToDeviceBuffer));
	RingBufferInit(&(cdc->deviceToHostBuffer));
	cdc->currentLineCoding->dwDTERRate = cdc->defaultLineCoding.dwDTERRate;
	cdc->currentLineCoding->bCharFormat = cdc->defaultLineCoding.bCharFormat;
	cdc->currentLineCoding->bParityType = cdc->defaultLineCoding.bParityType;
	cdc->currentLineCoding->bDataBits = cdc->defaultLineCoding.bDataBits;
}

void USBCDC_SendNetworkConnectionChange(USB_Device_Struct* device,
										const USB_Behaviour_Struct* behaviour,
										bool connected) {
	//TODO ***********
}
						
uint16_t USBCDC_ReadBytes(USB_Device_Struct* device,
						  const USBCDC_Behaviour_Struct* cdc,
						  uint8_t* buffer, uint16_t maxLen) {
	uint16_t read = RingBufferReadBuffer(&(cdc->hostToDeviceBuffer), buffer, maxLen);
	//we've freed some space in the ring buffer, trigger re-buffering check from host
	if (read > 0) USB_EP_TriggerInterrupt(device, cdc->dataOutEndpoint);
	return read;
}

uint16_t USBCDC_WriteBytes(USB_Device_Struct* device,
						  const USBCDC_Behaviour_Struct* cdc,
						  uint8_t* buffer, uint16_t maxLen) {
	uint16_t written = RingBufferWriteBuffer(&(cdc->deviceToHostBuffer), buffer, maxLen);
	//we've added some data to the ring buffer, trigger re-buffering check to host
	if (written > 0) USB_EP_TriggerInterrupt(device, cdc->dataInEndpoint);
	return written;
}


						

