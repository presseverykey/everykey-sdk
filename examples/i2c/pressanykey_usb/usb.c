#include "usb.h"
#include "usbsie.h"
#include "usbspec.h"

#include "../pressanykey/memorymap.h"
#include "../pressanykey/utils.h"
#include "../pressanykey/nvic.h"



#pragma mark USB globals

/** This is the only global - and it should only be used from interrupt handlers.
 * When this library is ported to another chip with multiple USB interfaces,
 * device-related info can be added into the device struct and
 * the interrupt can look up the corresponding device structure. This way,
 * the lib can be used with multiple USB instances simultaneously. */
USB_Device_Struct* _usbDevice;

#pragma mark SIE functions

void USB_SIE_Out(USB_Device_Struct* device, uint32_t cmdCode) {
	USB->DEVINTCLR =  USB_DEVINT_CC_EMPTY;		//clear cc empty interrupt flag
	USB->CMDCODE = cmdCode;						//send command
	while (!(USB->DEVINTST & (USB_DEVINT_CC_EMPTY | USB_DEVINT_DEV_STAT))) {}	//wait until cc is empty (command handled)
}

uint8_t USB_SIE_In(USB_Device_Struct* device, uint32_t cmdCode) {
	USB->DEVINTCLR =  USB_DEVINT_CC_EMPTY | USB_DEVINT_CD_FULL;	//clear cc empty and cd full interrupt flags
	USB->CMDCODE = cmdCode;						//send command
	while (!(USB->DEVINTST & (USB_DEVINT_CD_FULL | USB_DEVINT_DEV_STAT))) {}	//wait for cd full flag (data arrived)
	return USB->CMDDATA;						//get read byte
}

void USB_SIE_Command(USB_Device_Struct* device, USB_SIE_CommandID cmd) {
	USB_SIE_Out(device, cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
}

void USB_SIE_Command_Write1(USB_Device_Struct* device, USB_SIE_CommandID cmd, uint8_t value) {
	USB_SIE_Out(device, cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
	USB_SIE_Out(device, value << 16 | USB_CMDCODE_PHASE_WRITE);
}

uint8_t USB_SIE_Command_Read1(USB_Device_Struct* device, USB_SIE_CommandID cmd) {
	USB_SIE_Out(device, cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
	return USB_SIE_In(device, cmd << 16 | USB_CMDCODE_PHASE_READ);
}

uint16_t USB_SIE_Command_Read2(USB_Device_Struct* device, USB_SIE_CommandID cmd) {
	USB_SIE_Out(device, cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
	uint16_t value = USB_SIE_In(device, cmd << 16 | USB_CMDCODE_PHASE_READ);
	value += USB_SIE_In(device, cmd << 16 | USB_CMDCODE_PHASE_READ) << 8;
	return value;
}

void USB_SIE_SetConnected(USB_Device_Struct* device, bool connected) {
	USB_SIE_Command_Write1(device, USB_SIE_CMD_GetSetDeviceStatus, connected ? 1 : 0);
}

bool USB_SIE_GetConnected(USB_Device_Struct* device) {
	return (USB_SIE_Command_Read1(device, USB_SIE_CMD_GetSetDeviceStatus) & 1) ? true : false;
}

void USB_SIE_SetAddress(USB_Device_Struct* device, uint8_t address, bool enabled) {
	USB_SIE_Command_Write1(device, USB_SIE_CMD_SetAddress, address | (enabled ? 0x80 : 0));
}

void USB_SIE_ConfigureDevice(USB_Device_Struct* device, bool configured) {
	USB_SIE_Command_Write1(device, USB_SIE_CMD_ConfigureDevice, configured ? 1 : 0);
}

uint16_t USB_SIE_ReadInterruptStatus(USB_Device_Struct* device) {
	return USB_SIE_Command_Read2(device, USB_SIE_CMD_ReadInterruptStatus);
}

uint8_t USB_SIE_SelectEndpoint(USB_Device_Struct* device, uint8_t epIdx) {
	return USB_SIE_Command_Read1(device, USB_SIE_CMD_SelectEndpoint + epIdx);
}

uint8_t USB_SIE_SelectEndpointClearInterrupt(USB_Device_Struct* device, uint8_t epIdx) {
	return USB_SIE_Command_Read1(device, USB_SIE_CMD_SelectEndpointClearInterruptSetStatus + epIdx);
}

void USB_SIE_SetEndpointStatus(USB_Device_Struct* device, uint8_t epIdx, uint8_t status) {
	USB_SIE_Command_Write1(device, USB_SIE_CMD_SelectEndpointClearInterruptSetStatus + epIdx, status);
}

void USB_SIE_ClearBuffer(USB_Device_Struct* device, uint8_t epIdx) {
	if (epIdx > 9) return;
	USB_SIE_Command(device, USB_SIE_CMD_SelectEndpoint + epIdx);
	USB_SIE_Command(device, USB_SIE_CMD_ClearBuffer);
}

void USB_SIE_ValidateBuffer(USB_Device_Struct* device, uint8_t epIdx) {
	if (epIdx > 9) return;
	USB_SIE_Command(device, USB_SIE_CMD_SelectEndpoint + epIdx);
	USB_SIE_Command(device, USB_SIE_CMD_ValidateBuffer);
}

#pragma mark Endpoint functions

uint32_t USB_EP_Read(USB_Device_Struct* device, uint8_t epIdx, uint8_t* buffer, uint32_t maxLen) {
	if (!USB_EP_GetFull(device,epIdx)) return 0;				//cannot read - all buffers empty
	uint8_t logEpIdx = epIdx >> 1;
	USB->CTRL = USB_CTRL_LOG_EP * logEpIdx + USB_CTRL_RD_EN;	//we want to read the number of bytes available
	NOP;					//wait a bit to get len
	NOP;
	NOP;
	uint32_t readBytes;
	readBytes = USB->RXPLEN;
	if (readBytes & USB_RXPLEN_DV) {
		readBytes &= USB_RXPLEN_LENGTH_MASK;
		if (maxLen < readBytes) readBytes = maxLen;
		uint16_t readWords = (readBytes + 3) >> 2;				//round up, convert to words
		uint16_t i;
		for (i=0; i<readWords; i++) {
			((uint32_t*)buffer)[i] = USB->RXDATA;
		}
	} else readBytes = 0;
	USB->CTRL = 0;												//disable read again
	USB_SIE_ClearBuffer(device, epIdx);							//mark buffer as free again
	return readBytes;
}

uint32_t USB_EP_Write(USB_Device_Struct* device, uint8_t epIdx, const uint8_t* buffer, uint32_t length) {
	if (USB_EP_GetFull(device,epIdx)) return 0;					//cannot write - all buffers full
	//	if (USB_EP_GetStall(epIdx)) return length;					//EP is stalled: Do not write but flush output
	uint8_t logEpIdx = epIdx >> 1;
	USB->CTRL = USB_CTRL_LOG_EP * logEpIdx + USB_CTRL_WR_EN;	//enable writing for this ep
	NOP;					//wait a bit, just to be safe
	NOP;
	NOP;
	uint16_t epPacketSize = (epIdx < 8) ? USB_MAX_BULK_DATA_SIZE : USB_MAX_ISOCH_DATA_SIZE;
	if (epPacketSize < length) length = epPacketSize;
	USB->TXPLEN = length;
	NOP;					//wait a bit, just to be safe
	NOP;
	NOP;
	uint16_t writeWords = (length + 3) >> 2;
	uint16_t i;
	uint32_t* wordBuf = (uint32_t*)buffer;
	for (i = 0; i < writeWords; i++) {						//copy data out
		USB->TXDATA = wordBuf[i];
	}
	USB->CTRL = 0;											//turn off writing again
	USB_SIE_ValidateBuffer(device, epIdx);
	return length;
}

void USB_EP_SetStall(USB_Device_Struct* device, uint8_t epIdx, bool stalled) {
	USB_SIE_SetEndpointStatus(device, epIdx, stalled ? USB_EPSTAT_ST : 0);
}

bool USB_EP_GetStall(USB_Device_Struct* device, uint8_t epIdx) {
	uint8_t epStat = USB_SIE_SelectEndpoint(device, epIdx);
	return (epStat & USB_SELEP_ST) ? true : false;
}

bool USB_EP_GetFull(USB_Device_Struct* device, uint8_t epIdx) {
	uint8_t epStat = USB_SIE_SelectEndpoint(device, epIdx);
	return (epStat & USB_SELEP_FE) ? true : false;
}

void USB_EP_TriggerInterrupt(USB_Device_Struct* device, uint8_t epIdx) {
	USB->DEVINTSET = 2 << epIdx;
	NVIC_SetInterruptPending(NVIC_USBIRQ);
}


uint8_t USB_EP_LogicalToPhysicalIndex(uint8_t index) {
	return ((index & 0x0f) < 1) | (index & 0x80 >> 7);
}



#pragma mark USB common command handlers

void USB_Reset(USB_Device_Struct* device) {
	USB->DEVINTCLR = 0x3fff;			//clear all interrupts
	USB->DEVINTEN =
		USB_DEVINT_EP0 |
		USB_DEVINT_EP1 |
		USB_DEVINT_EP2 |
		USB_DEVINT_EP3 |
		USB_DEVINT_EP4 |
		USB_DEVINT_EP5 |
		USB_DEVINT_EP6 |
		USB_DEVINT_EP7 |
		USB_DEVINT_FRAME | 
		USB_DEVINT_DEV_STAT;				//enable EP interrupts + status change + frame
	device->usbSuspended = false;
	device->currentCommandDataBase = NULL;
	device->currentCommandDataRemaining = 0;
	device->currentConfiguration = 0;
	USB_SIE_SetAddress(device, 0, true);
	device->controlOutDataCompleteCallback = NULL;
	device->controlStatusCallback = NULL;
	int i;
	for (i=0; i<USB_MAX_INTERFACES_PER_DEVICE; i++) device->interfaceAltSetting[i] = 0;
}

void USB_Suspend(USB_Device_Struct* device) {
	device->usbSuspended = true;
}

void USB_Resume(USB_Device_Struct* device) {
	device->usbSuspended = false;
}

bool USB_HandleGetStatus(USB_Device_Struct* device) {
	if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) return false;
	if ((device->currentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_STANDARD) return false;
	if (device->currentCommand.wLengthH != 0) return false;
	if (device->currentCommand.wLengthL != 2) return false;
	if (device->currentCommand.wValueL != 0) return false;
	if (device->currentCommand.wValueH != 0) return false;
	
	uint16_t index = (device->currentCommand.wIndexH << 8) | device->currentCommand.wIndexL;

	switch (device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
		case USB_RT_RECIPIENT_DEVICE:
			if (index != 0) return false;
			device->commandDataBuffer[0] = 0;
			break;
		case USB_RT_RECIPIENT_INTERFACE:
			device->commandDataBuffer[0] = 0;
			break;
		case USB_RT_RECIPIENT_ENDPOINT:
		{
			uint8_t epIdx = USB_EP_LogicalToPhysicalIndex(index);	//convert USB ep address to physical address
			if (epIdx > 9) return false;	//out of range
			device->commandDataBuffer[0] = USB_EP_GetStall(device, epIdx) ? 1 : 0;
		}
			break;
		default:
			return false;
	}			
	device->commandDataBuffer[1] = 0;
	device->currentCommandDataBase = device->commandDataBuffer;
	device->currentCommandDataRemaining = 2;
	return true;
}

bool USB_HandleGetDescriptor(USB_Device_Struct* device) {
	//Request only sent to device. wValueH contains the descriptor type, wValueL contains the desriptor index.
	//In case of string descriptors, wIndex (l and h) contains the language ID
	if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_DEVICE) return false;
	if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) return false;
	const uint8_t* base = NULL;
	uint16_t len = 0;
	switch (device->currentCommand.wValueH) {
		case USB_DESC_DEVICE:
			base = device->deviceDefinition->deviceDescriptor;
			len = base[0];
			break;
		case USB_DESC_CONFIGURATION:
			if (device->currentCommand.wValueL < device->deviceDefinition->configurationCount) {
				base = device->deviceDefinition->configurationDescriptors[device->currentCommand.wValueL];
				len = (base[3] << 8) | base[2];
			}
			break;
		case USB_DESC_STRING:
			if (device->currentCommand.wValueL < device->deviceDefinition->stringCount) {
				base = device->deviceDefinition->strings[device->currentCommand.wValueL];
				len =  base[0];
			}
			break;
	}
	if (base == NULL) return false;
	uint16_t maxLen = ((device->currentCommand.wLengthH) << 8) | (device->currentCommand.wLengthL);
	if (len > maxLen) len = maxLen;
	/* Allow assignment to non-const pointer. We already checked that the direction is DEVICE_TO_HOST,
	 so that buffer contents are not modified. We could also have different base pointers, but we want
	 to save memory. */
	device->currentCommandDataBase = (uint8_t*)base;	
	device->currentCommandDataRemaining = len;
	return true;
}


bool USB_HandleClearFeature(USB_Device_Struct* device) {
	if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) return false;
	if ((device->currentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_STANDARD) return false;
	if (device->currentCommand.wLengthH != 0) return false;
	if (device->currentCommand.wLengthL != 0) return false;
	int wIndex = (device->currentCommand.wIndexH << 8) | device->currentCommand.wIndexL;
	int wValue = (device->currentCommand.wValueH << 8) | device->currentCommand.wValueL;
	switch (device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
		case USB_RT_RECIPIENT_DEVICE:
			//remote wakeup, test mode
			//TODO
			return true;
			break;
		case USB_RT_RECIPIENT_INTERFACE:
			//no features to clear
			break;
		case USB_RT_RECIPIENT_ENDPOINT:
				if (wValue == USB_FEATURE_ENDPOINT_HALT) {
					USB_EP_SetStall(device, USB_EP_LogicalToPhysicalIndex(wIndex),false);
					return true;
				}
			break;
		default:
				break;
	}
	return true;
}

bool USB_HandleSetFeature(USB_Device_Struct* device) {
	return false;	//TODO
}

bool USB_HandleGetConfiguration(USB_Device_Struct* device) {
	if (device->currentCommand.wValueL != 0) return false;
	if (device->currentCommand.wValueH != 0) return false;
	if (device->currentCommand.wIndexL != 0) return false;
	if (device->currentCommand.wIndexH != 0) return false;
	uint16_t len = 1;
	uint16_t maxLen = ((device->currentCommand.wLengthH) << 8) | (device->currentCommand.wLengthL);
	if (len > maxLen) len = maxLen;
	/* Allow assignment to non-const pointer. We already checked that the direction is DEVICE_TO_HOST,
	 so that buffer contents are not modified. We could also have different base pointers, but we want
	 to save memory. */
	device->currentCommandDataBase = (&(device->currentConfiguration));
	device->currentCommandDataRemaining = len;
	return true;
}

bool USB_HandleSetConfiguration(USB_Device_Struct* device) {
	if (device->currentCommand.wIndexL != 0) return false;
	if (device->currentCommand.wIndexH != 0) return false;
	if (device->currentCommand.wLengthL != 0) return false;
	if (device->currentCommand.wLengthL != 0) return false;
	if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_DEVICE) return false;
	device->currentConfiguration = device->currentCommand.wValueL;
	USB_SIE_ConfigureDevice(device, device->currentConfiguration != 0);

	uint8_t i;
	for (i=0; i<device->deviceDefinition->behaviourCount; i++) {
		const USB_Behaviour_Struct* behaviour = device->deviceDefinition->behaviours[i];
		USBConfigChangeCallback cb = behaviour->configChangeCallback;
		if (cb) cb(device, behaviour);
	}
	return true;
}

bool USB_HandleSetAddress2(USB_Device_Struct* device) {
	USB_SIE_SetAddress(device, device->currentCommand.wValueL & 0x7f, 1);
	return true;
}

bool USB_HandleSetAddress(USB_Device_Struct* device) {
	if (device->currentCommand.bmRequestType != 0) return false;
	if (device->currentCommand.wIndexL != 0) return false;
	if (device->currentCommand.wIndexH != 0) return false;
	if (device->currentCommand.wLengthL != 0) return false;
	if (device->currentCommand.wLengthH != 0) return false;
	if (device->currentCommand.wValueH != 0) return false;
	if (device->currentCommand.wValueL >= 0x80) return false;
	device->controlStatusCallback = USB_HandleSetAddress2; //we actually change address after status out
	return true;
}

bool USB_HandleGetInterface(USB_Device_Struct* device) {
	if (device->currentCommand.bmRequestType != (USB_RT_DIR_DEVICE_TO_HOST | USB_RT_RECIPIENT_INTERFACE)) return false;
	if (device->currentCommand.wValueH != 0) return false;
	if (device->currentCommand.wValueL != 0) return false;
	if (device->currentCommand.wIndexH != 0) return false;
	if (device->currentCommand.wIndexL >= USB_MAX_INTERFACES_PER_DEVICE) return false;
	if (device->currentCommand.wLengthH != 0) return false;
	if (device->currentCommand.wLengthL != 1) return false;
	device->commandDataBuffer[0] = device->interfaceAltSetting[device->currentCommand.wIndexL];
	device->currentCommandDataBase = device->commandDataBuffer;
	device->currentCommandDataRemaining = 1;
	return true;
}

bool USB_HandleSetInterface(USB_Device_Struct* device) {
/*	TODO: Put this back in and check if it works, was temporarily taken out ***************
	if (device->currentCommand.bmRequestType != (USB_RT_DIR_HOST_TO_DEVICE | USB_RT_RECIPIENT_INTERFACE)) return false;
	if (device->currentCommand.wValueH != 0) return false;
	if (device->currentCommand.wIndexH != 0) return false;
	if (device->currentCommand.wIndexL >= USB_MAX_INTERFACES_PER_DEVICE) return false;
	if (device->currentCommand.wLengthH != 0) return false;
	if (device->currentCommand.wLengthL != 0) return false;
*/	
	uint8_t i;
	for (i=0; i<device->deviceDefinition->behaviourCount; i++) {
		const USB_Behaviour_Struct* behaviour = device->deviceDefinition->behaviours[i];
		USBInterfaceAltCallback cb = behaviour->interfaceAltCallback;
		if (cb && cb(device, behaviour, device->currentCommand.wIndexL, device->currentCommand.wValueL)) break;
	}
	if (i >= device->deviceDefinition->behaviourCount) return false;	//none of them accepted it
	
	device->interfaceAltSetting[device->currentCommand.wIndexL] = device->currentCommand.wValueL;
	return true;
}

#pragma mark USB control and data flow handling

void USB_Control_ReceiveDeviceToHostStatus(USB_Device_Struct* device) {
	USB_EP_Read(device, 0, NULL, 0);
	if (device->controlStatusCallback) {
		if (!(device->controlStatusCallback(device))) {
			USB_EP_SetStall(device, 1, true);
		}
	}
	device->controlStatusCallback = NULL;
}

void USB_Control_ReceiveHostToDeviceData(USB_Device_Struct* device) {
	uint32_t read = USB_EP_Read(device, 0,
								device->currentCommandDataBase,
								device->currentCommandDataRemaining);
	device->currentCommandDataRemaining -= read;
	device->currentCommandDataBase += read;
}

void USB_Control_WriteDeviceToHostData(USB_Device_Struct* device) {
	uint32_t written = USB_EP_Write(device, 1,
									device->currentCommandDataBase,
									device->currentCommandDataRemaining);
	device->currentCommandDataBase += written;
	device->currentCommandDataRemaining -= written;
}

void USB_Control_WriteHostToDeviceStatus(USB_Device_Struct* device) {
	USB_EP_Write(device, 1, NULL, 0);
	if (device->controlStatusCallback) device->controlStatusCallback(device);
	device->controlStatusCallback = NULL;
}

/** A command has arrived on the control endpoint. Dispatch by command. */
void USB_Control_HandleSetup(USB_Device_Struct* device) {
	USB_EP_Read(device, 0,(uint8_t*)(&(device->currentCommand)),sizeof(USB_Setup_Packet));

	device->currentCommandDataBase = NULL;
	device->currentCommandDataRemaining = 0;
	device->controlOutDataCompleteCallback = NULL;
	device->controlStatusCallback = NULL;
	device->callbackRefcon = NULL;
	
	bool handled = false;
	uint8_t i;
	for (i=0; i<device->deviceDefinition->behaviourCount; i++) {
		const USB_Behaviour_Struct* behaviour = device->deviceDefinition->behaviours[i];
		USBExtendedControlSetupCallback cb = behaviour->extendedControlSetupCallback;
		if (cb) handled = (*cb)(device, behaviour);
		if (handled) break;	
	}
	if (!handled) {
		switch (device->currentCommand.bRequest) {
			case USB_REQ_GET_STATUS:
				handled = USB_HandleGetStatus(device);
				break;
			case USB_REQ_CLEAR_FEATURE:
				handled = USB_HandleClearFeature(device);
				break;
			case USB_REQ_SET_FEATURE:
				handled = USB_HandleClearFeature(device);
				break;
			case USB_REQ_SET_ADDRESS:
				handled = USB_HandleSetAddress(device);
				break;
			case USB_REQ_GET_DESCRIPTOR:
				handled = USB_HandleGetDescriptor(device);
				break;
			case USB_REQ_GET_CONFIGURATION:
				handled = USB_HandleGetConfiguration(device);
				break;
			case USB_REQ_SET_CONFIGURATION:
				handled = USB_HandleSetConfiguration(device);
				break;
			case USB_REQ_GET_INTERFACE:
				handled = USB_HandleGetInterface(device);
				break;
			case USB_REQ_SET_INTERFACE:
				handled = USB_HandleSetInterface(device);
				break;
			default:
				break;
		}
	}
	if (handled) {
		bool deviceToHost = device->currentCommand.bmRequestType & USB_RT_DIR_DEVICE_TO_HOST;
		bool haveData = device->currentCommandDataRemaining > 0;
		if (deviceToHost && haveData) USB_Control_WriteDeviceToHostData(device);
		else if (!deviceToHost && !haveData) USB_Control_WriteHostToDeviceStatus(device);
	} else {
		USB_EP_SetStall(device, 1, true);
	}
}

/** data arrived for us - either HostToDevice command data stage or DeviceToHost status stage */
void USB_Control_HandleOut(USB_Device_Struct* device) { 
	bool deviceToHost = device->currentCommand.bmRequestType & USB_RT_DIR_DEVICE_TO_HOST;
	if (deviceToHost) USB_Control_ReceiveDeviceToHostStatus(device);
	else {
		if (device->currentCommandDataRemaining > 0) {
			USB_Control_ReceiveHostToDeviceData(device);
			if (device->currentCommandDataRemaining == 0) {
				bool success = true;
				if (device->controlOutDataCompleteCallback) {
					success = device->controlOutDataCompleteCallback(device);
					device->controlOutDataCompleteCallback = NULL;
				}
				if (success) USB_Control_WriteHostToDeviceStatus(device);
				else USB_EP_SetStall(device, 1, true);
			}
		} else {
			USB_EP_SetStall(device, 1, true);
		}
	}
}

/** buffer for sending is free again - either DeviceToHost data stage or HostToDevice status stage */
void USB_Control_HandleIn(USB_Device_Struct* device) { 
	bool deviceToHost = device->currentCommand.bmRequestType & USB_RT_DIR_DEVICE_TO_HOST;
	if (deviceToHost) USB_Control_WriteDeviceToHostData(device);
	else USB_Control_WriteHostToDeviceStatus(device);
}


/** Handle data on a non-control endpoint (in data available or out data sent) */
void USB_HandleData(USB_Device_Struct* device, int epIdx) {
	uint8_t i;
	for (i=0; i<device->deviceDefinition->behaviourCount; i++) {
		const USB_Behaviour_Struct* behaviour = device->deviceDefinition->behaviours[i];
		USBEndpointDataCallback cb = behaviour->endpointDataCallback;
		if (cb && cb(device, behaviour, epIdx)) break;
	}
	if (i >= device->deviceDefinition->behaviourCount) {
		//No behaviour handled this data message - stall endpoint
		USB_EP_SetStall(device, epIdx, true);
	}
}

/** this function is added to the interrupt vector table - see startup.c */
void usb_irq_handler(void) {
	USB_Device_Struct* device = _usbDevice;
	uint32_t interruptMask = USB->DEVINTST;	//read interrupt pending mask
	USB->DEVINTCLR = interruptMask;			//clear interrupt pending mask
	
	//We could test other USB interrupts here if we need to

	//Check frame interrupt
	if (interruptMask & USB_DEVINT_FRAME) {
		uint8_t i;
		for (i=0; i<device->deviceDefinition->behaviourCount; i++) {
			const USB_Behaviour_Struct* behaviour = device->deviceDefinition->behaviours[i];
			USBFrameCallback cb = behaviour->frameCallback;
			if (cb) cb(device, behaviour);
		}
	}

	//Check endpoint interrupts
	int epIdx;
	for (epIdx = 0; epIdx < 8; epIdx++) {
		uint32_t epIntMask = 2 << epIdx;
		if (interruptMask & epIntMask) {
			uint8_t epStat = USB_SIE_SelectEndpointClearInterrupt(device, epIdx);	//Clear interrupt in SIE
			switch (epIdx) {
				case 0:
					if (epStat & USB_SELEP_STP) USB_Control_HandleSetup(device);
					else USB_Control_HandleOut(device);
					break;
				case 1:
					USB_Control_HandleIn(device);
					break;
				default:
					USB_HandleData(device, epIdx);
					break;
			}
		}
	}	
}

#pragma mark USB high level API

void USB_Init(const USB_Device_Definition* definition, USB_Device_Struct* device) {

	device->deviceDefinition = definition;
	_usbDevice = device;
	
	// Turn AHB clock for peripherals: GPIO, IOCON and USB_REG
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_GPIO | SYSCON_SYSAHBCLKCTRL_IOCON | SYSCON_SYSAHBCLKCTRL_USB_REG;

	// Configure USB clock
 	SYSCON->PDRUNCFG &= ~(SYSCON_USBPLL_PD | SYSCON_USBPAD_PD);	//Turn on USB PLL and PHY 
	SYSCON->USBPLLCLKSEL = 1;			//choose system oscillator for USB PLL
	SYSCON->USBPLLCLKUEN = 0;			//Trigger USB PLL source change
	SYSCON->USBPLLCLKUEN = 1;
	SYSCON->USBPLLCTRL = 0b0100011;			//Fin=12MHz, Fout=48MHz -> M=4, P=2
	while (! (SYSCON->USBPLLSTAT & 1)) {};		//wait for PLL to lock
	SYSCON->USBCLKSEL = 0;				//USB clock: USB PLL out
	SYSCON->USBCLKUEN = 0;				//Trigger main clock source change
	SYSCON->USBCLKUEN = 1;

	// Configure pins
	IOCON->PIO0_3 = 0xc1;				//set PIO0_3 to VBus function
	IOCON->PIO0_6 = 0xc1;				//set PIO0_6 to !USB_Connect function

	// Enable interrupts
	NVIC_EnableInterrupt(NVIC_USBFIQ);
	NVIC_EnableInterrupt(NVIC_USBIRQ);
	
	USB_Reset(device); //set all state variables to start
}

void USB_SoftConnect(USB_Device_Struct* device) {
	USB_SIE_SetConnected(device, true);
}

void USB_SoftDisconnect(USB_Device_Struct* device) {
	USB_SIE_SetConnected(device, false);
}

bool USB_Connected(USB_Device_Struct* device) {
	return USB_SIE_GetConnected(device);
}


