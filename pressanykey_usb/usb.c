#include "usb.h"
#include "usbsie.h"
#include "usbspec.h"

#include "../pressanykey/memorymap.h"
#include "../pressanykey/utils.h"
#include "../pressanykey/nvic.h"


#pragma mark USB globals

bool usbSuspended;
USB_Device_Struct* usbDeviceDefinition;
uint16_t usbHaltedEndpointMask;
uint8_t usbCommandDataBuffer[USB_MAX_COMMAND_PACKET_SIZE];
uint8_t usbConfiguration;

USB_Setup_Packet usbCurrentCommand;
uint8_t* usbCurrentCommandDataBase;			//Don't modify contents if current command is USB_RT_DIR_DEVICE_TO_HOST
uint32_t usbCurrentCommandDataRemaining;

void (*usbControlOutDataCompleteCallback)(void);	//callback when host-to-device has arrived
void (*usbControlStatusCallback)(void);				//callback when status was sent


#pragma mark SIE functions

void USB_SIE_Out(uint32_t cmdCode) {
	USB->DEVINTCLR =  USB_DEVINT_CC_EMPTY;		//clear cc empty interrupt flag
	USB->CMDCODE = cmdCode;						//send command
	while (!(USB->DEVINTST & (USB_DEVINT_CC_EMPTY | USB_DEVINT_DEV_STAT))) {}	//wait until cc is empty (command handled)
}

uint8_t USB_SIE_In(uint32_t cmdCode) {
	USB->DEVINTCLR =  USB_DEVINT_CC_EMPTY | USB_DEVINT_CD_FULL;	//clear cc empty and cd full interrupt flags
	USB->CMDCODE = cmdCode;						//send command
	while (!(USB->DEVINTST & (USB_DEVINT_CD_FULL | USB_DEVINT_DEV_STAT))) {}	//wait for cd full flag (data arrived)
	return USB->CMDDATA;						//get read byte
}

void USB_SIE_Command(USB_SIE_CommandID cmd) {
	USB_SIE_Out(cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
}

void USB_SIE_Command_Write1(USB_SIE_CommandID cmd, uint8_t value) {
	USB_SIE_Out(cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
	USB_SIE_Out(value << 16 | USB_CMDCODE_PHASE_WRITE);
}

uint8_t USB_SIE_Command_Read1(USB_SIE_CommandID cmd) {
	USB_SIE_Out(cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
	return USB_SIE_In(cmd << 16 | USB_CMDCODE_PHASE_READ);
}

uint16_t USB_SIE_Command_Read2(USB_SIE_CommandID cmd) {
	USB_SIE_Out(cmd << 16 | USB_CMDCODE_PHASE_COMMAND);
	uint16_t value = USB_SIE_In(cmd << 16 | USB_CMDCODE_PHASE_READ);
	value += USB_SIE_In(cmd << 16 | USB_CMDCODE_PHASE_READ) << 8;
	return value;
}

void USB_SIE_SetConnected(bool connected) {
	USB_SIE_Command_Write1(USB_SIE_CMD_GetSetDeviceStatus, connected ? 1 : 0);
}

bool USB_SIE_GetConnected(void) {
	return (USB_SIE_Command_Read1(USB_SIE_CMD_GetSetDeviceStatus) & 1) ? true : false;
}

void USB_SIE_SetAddress(uint8_t address, bool enabled) {
	USB_SIE_Command_Write1(USB_SIE_CMD_SetAddress, address | (enabled ? 0x80 : 0));
}

void USB_SIE_ConfigureDevice(bool configured) {
	USB_SIE_Command_Write1(USB_SIE_CMD_ConfigureDevice, configured ? 1 : 0);
}

uint16_t USB_SIE_ReadInterruptStatus() {
	return USB_SIE_Command_Read2(USB_SIE_CMD_ReadInterruptStatus);
}

uint8_t USB_SIE_SelectEndpoint(uint8_t epIdx) {
	return USB_SIE_Command_Read1(USB_SIE_CMD_SelectEndpoint + epIdx);
}

uint8_t USB_SIE_SelectEndpointClearInterrupt(uint8_t epIdx) {
	return USB_SIE_Command_Read1(USB_SIE_CMD_SelectEndpointClearInterruptSetStatus + epIdx);
}

void USB_SIE_SetEndpointStatus(uint8_t epIdx, uint8_t status) {
	USB_SIE_Command_Write1(USB_SIE_CMD_SelectEndpointClearInterruptSetStatus + epIdx, status);
}

void USB_SIE_ClearBuffer(uint8_t epIdx) {
	if (epIdx > 7) return;		//isoch endpoint
	USB_SIE_Command(USB_SIE_CMD_SelectEndpoint + epIdx);
	USB_SIE_Command(USB_SIE_CMD_ClearBuffer);
}

void USB_SIE_ValidateBuffer(uint8_t epIdx) {
	if (epIdx > 7) return;		//isoch endpoint
	USB_SIE_Command(USB_SIE_CMD_SelectEndpoint + epIdx);
	USB_SIE_Command(USB_SIE_CMD_ValidateBuffer);
}

#pragma mark Endpoint functions

uint32_t USB_EP_Read(uint8_t epIdx, uint8_t* buffer, uint32_t maxLen) {
	uint8_t logEpIdx = epIdx >> 1;
	USB->CTRL = USB_CTRL_LOG_EP * logEpIdx + USB_CTRL_RD_EN;	//we want to read the number of bytes available
	NOP;					//wait a bit to get len
	NOP;
	NOP;
	uint32_t readBytes;
	do {
		readBytes = USB->RXPLEN;
	} while (!(readBytes & USB_RXPLEN_DV));
	readBytes &= USB_RXPLEN_LENGTH_MASK;
	if (maxLen < readBytes) readBytes = maxLen;
	uint16_t readWords = (readBytes + 3) >> 2;				//round up, convert to words
	uint16_t i;
	for (i=0; i<readWords; i++) {
		((uint32_t*)buffer)[i] = USB->RXDATA;
	}
	USB->CTRL = 0;											//disable read again
	USB_SIE_ClearBuffer(epIdx);								//mark buffer as free again
	return readBytes;
}

uint32_t USB_EP_Write(uint8_t epIdx, const uint8_t* buffer, uint32_t length) {
//	if (USB_EP_GetStall(epIdx)) return length; 				//EP is stalled: Do not write but flush output
	uint8_t logEpIdx = epIdx >> 1;
	USB->CTRL = USB_CTRL_LOG_EP * logEpIdx + USB_CTRL_WR_EN;	//enable writing for this ep
	NOP;					//wait a bit, just to be safe
	NOP;
	NOP;
	uint16_t epPacketSize = (epIdx < 8) ? 64 : 512;
	NOP;					//wait a bit, just to be safe
	NOP;
	NOP;
	if (epPacketSize < length) length = epPacketSize;
	USB->TXPLEN = length;
	uint16_t writeWords = (length + 3) >> 2;
	uint16_t i;
	uint32_t* wordBuf = (uint32_t*)buffer;
	for (i = 0; i < writeWords; i++) {						//copy data out
		USB->TXDATA = wordBuf[i];
	}
	USB->CTRL = 0;											//turn off writing again
	USB_SIE_ValidateBuffer(epIdx);
	return length;
}

void USB_EP_SetStall(uint8_t epIdx, bool stalled) {
	USB_SIE_SetEndpointStatus(epIdx, stalled ? USB_EPSTAT_ST : 0);
}

bool USB_EP_GetStall(uint8_t epIdx) {
	uint8_t epStat = USB_SIE_SelectEndpoint(epIdx);
	return (epStat & USB_SELEP_ST) ? true : false;
}

uint8_t USB_EP_LogicalToPhysicalIndex(uint8_t index) {
	return ((index & 0x0f) < 1) | (index & 0x80 >> 7);
}

#pragma mark USB common command handlers

void USB_Reset() {
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
		USB_DEVINT_DEV_STAT;				//enable EP interrupts + status change
	usbSuspended = false;
	usbCurrentCommandDataBase = NULL;
	usbCurrentCommandDataRemaining = 0;
	usbConfiguration = 0;
	USB_SIE_SetAddress(0, true);
	usbControlOutDataCompleteCallback = NULL;
	usbControlStatusCallback = NULL;
}

void USB_Suspend() {
	usbSuspended = true;
}

void USB_Resume() {
	usbSuspended = false;
}

bool USB_HandleGetStatus() {
	if ((usbCurrentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) return false;
	if ((usbCurrentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_STANDARD) return false;
	if (usbCurrentCommand.wLengthH != 0) return false;
	if (usbCurrentCommand.wLengthL != 2) return false;
	if (usbCurrentCommand.wValueL != 0) return false;
	if (usbCurrentCommand.wValueH != 0) return false;
	
	uint16_t index = (usbCurrentCommand.wIndexH << 8) | usbCurrentCommand.wIndexL;

	switch (usbCurrentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
		case USB_RT_RECIPIENT_DEVICE:
			if (index != 0) return false;
			usbCommandDataBuffer[0] = 0;
			break;
		case USB_RT_RECIPIENT_INTERFACE:
			usbCommandDataBuffer[0] = 0;
			break;
		case USB_RT_RECIPIENT_ENDPOINT:
		{
			uint8_t epIdx = USB_EP_LogicalToPhysicalIndex(index);	//convert USB ep address to physical address
			if (epIdx > 9) return false;	//out of range
			usbCommandDataBuffer[0] = USB_EP_GetStall(epIdx) ? 1 : 0;
		}
			break;
		default:
			return false;
	}			
	usbCommandDataBuffer[1] = 0;
	usbCurrentCommandDataBase = usbCommandDataBuffer;
	usbCurrentCommandDataRemaining = 2;
	return true;
}

bool USB_HandleGetDescriptor() {
	//Request only sent to device. wValueH contains the descriptor type, wValueL contains the desriptor index.
	//In case of string descriptors, wIndex (l and h) contains the language ID
	if ((usbCurrentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_DEVICE) return false;
	if ((usbCurrentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_DEVICE_TO_HOST) return false;
	const uint8_t* base = NULL;
	uint16_t len = 0;
	switch (usbCurrentCommand.wValueH) {
		case USB_DESC_DEVICE:
			base = usbDeviceDefinition->deviceDescriptor;
			len = base[0];
			break;
		case USB_DESC_CONFIGURATION:
			if (usbCurrentCommand.wValueL < usbDeviceDefinition->configurationCount) {
				base = usbDeviceDefinition->configurationDescriptors[usbCurrentCommand.wValueL];
				len = (base[3] << 8) | base[2];
			}
			break;
		case USB_DESC_STRING:
			if (usbCurrentCommand.wValueL < usbDeviceDefinition->stringCount) {
				base = usbDeviceDefinition->strings[usbCurrentCommand.wValueL];
				len =  base[0];
			}
			break;
	}
	if (base == NULL) return false;
	uint16_t maxLen = ((usbCurrentCommand.wLengthH) << 8) | (usbCurrentCommand.wLengthL);
	if (len > maxLen) len = maxLen;
	/* Allow assignment to non-const pointer. We already checked that the direction is DEVICE_TO_HOST,
	 so that buffer contents are not modified. We could also have different base pointers, but we want
	 to save memory. */
	usbCurrentCommandDataBase = (uint8_t*)base;	
	usbCurrentCommandDataRemaining = len;
	return true;
}


bool USB_HandleClearFeature() {
	if ((usbCurrentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) return false;
	if ((usbCurrentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_STANDARD) return false;
	if (usbCurrentCommand.wLengthH != 0) return false;
	if (usbCurrentCommand.wLengthL != 0) return false;
	int wIndex = (usbCurrentCommand.wIndexH << 8) | usbCurrentCommand.wIndexL;
	int wValue = (usbCurrentCommand.wValueH << 8) | usbCurrentCommand.wValueL;
	switch (usbCurrentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
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
					USB_EP_SetStall(USB_EP_LogicalToPhysicalIndex(wIndex),false);
					return true;
				}
			break;
		default:
				break;
	}
	return true;
}

bool USB_HandleSetFeature() {
	return false;	//TODO
}

bool USB_HandleGetConfiguration() {
	if (usbCurrentCommand.wValueL != 0) return false;
	if (usbCurrentCommand.wValueH != 0) return false;
	if (usbCurrentCommand.wIndexL != 0) return false;
	if (usbCurrentCommand.wIndexH != 0) return false;
	uint16_t len = 1;
	uint16_t maxLen = ((usbCurrentCommand.wLengthH) << 8) | (usbCurrentCommand.wLengthL);
	if (len > maxLen) len = maxLen;
	/* Allow assignment to non-const pointer. We already checked that the direction is DEVICE_TO_HOST,
	 so that buffer contents are not modified. We could also have different base pointers, but we want
	 to save memory. */
	usbCurrentCommandDataBase = (&usbConfiguration);
	usbCurrentCommandDataRemaining = len;
	return true;
}

bool USB_HandleSetConfiguration() {
	if (usbCurrentCommand.wIndexL != 0) return false;
	if (usbCurrentCommand.wIndexH != 0) return false;
	if (usbCurrentCommand.wLengthL != 0) return false;
	if (usbCurrentCommand.wLengthL != 0) return false;
	if ((usbCurrentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_DEVICE) return false;
	usbConfiguration = usbCurrentCommand.wValueL;
	USB_SIE_ConfigureDevice(usbConfiguration != 0);
	return true;
}

void USB_HandleSetAddress2() {
	USB_SIE_SetAddress(usbCurrentCommand.wValueL & 0x7f, 1);
}

bool USB_HandleSetAddress() {
	if (usbCurrentCommand.bmRequestType != 0) return false;
	if (usbCurrentCommand.wIndexL != 0) return false;
	if (usbCurrentCommand.wIndexH != 0) return false;
	if (usbCurrentCommand.wLengthL != 0) return false;
	if (usbCurrentCommand.wLengthH != 0) return false;
	if (usbCurrentCommand.wValueH != 0) return false;
	if (usbCurrentCommand.wValueL >= 0x80) return false;
	usbControlStatusCallback = USB_HandleSetAddress2; //we actually change address after status out
	return true;
}

#pragma mark USB control and data flow handling

void USB_Control_ReceiveDeviceToHostStatus() {
	USB_EP_Read(0, NULL, 0);
	if (usbControlStatusCallback) usbControlStatusCallback();
	usbControlStatusCallback = NULL;
}

void USB_Control_ReceiveHostToDeviceData() {
	uint32_t read = USB_EP_Read(0, usbCurrentCommandDataBase, usbCurrentCommandDataRemaining);
	usbCurrentCommandDataRemaining -= read;
	usbCurrentCommandDataBase += read;
}

void USB_Control_WriteDeviceToHostData() {
	uint32_t written = USB_EP_Write(1, usbCurrentCommandDataBase, usbCurrentCommandDataRemaining);
	usbCurrentCommandDataBase += written;
	usbCurrentCommandDataRemaining -= written;
}

void USB_Control_WriteHostToDeviceStatus() {
	USB_EP_Write(1, NULL, 0);
	if (usbControlStatusCallback) usbControlStatusCallback();
	usbControlStatusCallback = NULL;
}

/** A command has arrived on the control endpoint. Dispatch by command. */
void USB_Control_HandleSetup() {
	USB_EP_Read(0,(uint8_t*)(&usbCurrentCommand),sizeof(USB_Setup_Packet));
	usbCurrentCommandDataBase = NULL;
	usbCurrentCommandDataRemaining = 0;
	usbControlOutDataCompleteCallback = NULL;
	usbControlStatusCallback = NULL;
	bool ok = false;
	if (usbDeviceDefinition->extendedControlSetupCallback) {
		ok = usbDeviceDefinition->extendedControlSetupCallback();
	}
	if (!ok) {
		switch (usbCurrentCommand.bRequest) {
			case USB_REQ_GET_STATUS:
				ok = USB_HandleGetStatus();
				break;
			case USB_REQ_CLEAR_FEATURE:
				ok = USB_HandleClearFeature();
				break;
			case USB_REQ_SET_FEATURE:
				ok = USB_HandleClearFeature();
				break;
			case USB_REQ_SET_ADDRESS:
				ok = USB_HandleSetAddress();
				break;
			case USB_REQ_GET_DESCRIPTOR:
				ok = USB_HandleGetDescriptor();
				break;
			case USB_REQ_GET_CONFIGURATION:
				ok = USB_HandleGetConfiguration();
				break;
			case USB_REQ_SET_CONFIGURATION:
				ok = USB_HandleSetConfiguration();
				break;
			default:
				break;
		}
	}
	if (ok) {
		bool deviceToHost = usbCurrentCommand.bmRequestType & USB_RT_DIR_DEVICE_TO_HOST;
		bool haveData = usbCurrentCommandDataRemaining > 0;
		if (deviceToHost && haveData) USB_Control_WriteDeviceToHostData();
		else if (!deviceToHost && !haveData) USB_Control_WriteHostToDeviceStatus();
	} else {
		USB_EP_SetStall(1,true);
	}
}

/** data arrived for us - either HostToDevice command data stage or DeviceToHost status stage */
void USB_Control_HandleOut() { 
	bool deviceToHost = usbCurrentCommand.bmRequestType & USB_RT_DIR_DEVICE_TO_HOST;
	if (deviceToHost) USB_Control_ReceiveDeviceToHostStatus();
	else {
		if (usbCurrentCommandDataRemaining > 0) {
			USB_Control_ReceiveHostToDeviceData();
			if (usbCurrentCommandDataRemaining == 0) {
				if (usbControlOutDataCompleteCallback) usbControlOutDataCompleteCallback();
				usbControlStatusCallback = NULL;
				USB_Control_WriteHostToDeviceStatus();
			}
		} else {
			USB_EP_SetStall(1,true);
		}
	}
}

/** buffer for sending is free again - either DeviceToHost data stage or HostToDevice status stage */
void USB_Control_HandleIn() { 
	bool deviceToHost = usbCurrentCommand.bmRequestType & USB_RT_DIR_DEVICE_TO_HOST;
	if (deviceToHost) USB_Control_WriteDeviceToHostData();
	else USB_Control_WriteHostToDeviceStatus();
}


/** Handle data on a non-control endpoint */
void USB_HandleData(int epIdx) {
	if (usbDeviceDefinition->endpointDataCallback) {
		usbDeviceDefinition->endpointDataCallback(epIdx);
	}
	else USB_EP_SetStall(epIdx, true);
}

/** this function is added to the interrupt vector table - see startup.c */
void usb_irq_handler(void) {
	uint32_t interruptMask = USB->DEVINTST;	//read interrupt pending mask
	USB->DEVINTCLR = interruptMask;			//clear interrupt pending mask
	
	//We could test other USB interrupts here, but we don't need do

	int epIdx;
	for (epIdx = 0; epIdx < 8; epIdx++) {
		uint32_t epIntMask = 2 << epIdx;
		if (interruptMask & epIntMask) {
			uint8_t epStat = USB_SIE_SelectEndpointClearInterrupt(epIdx);	//Clear interrupt in SIE
			switch (epIdx) {
				case 0:
					if (epStat & USB_SELEP_STP) USB_Control_HandleSetup();
					else USB_Control_HandleOut();
					break;
				case 1:
					USB_Control_HandleIn();
					break;
				default:
					USB_HandleData(epIdx);
					break;
			}
		}
	}	
}

#pragma mark USB high level API

void USB_Init(USB_Device_Struct* deviceDefinition) {
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
	
	usbDeviceDefinition = deviceDefinition;

	USB_Reset(); //set all state variables to start
}

void USB_SoftConnect(void) {
	USB_SIE_SetConnected(true);
}

void USB_SoftDisconnect(void) {
	USB_SIE_SetConnected(false);
}

bool USB_Connected(void) {
	return USB_SIE_GetConnected();
}


