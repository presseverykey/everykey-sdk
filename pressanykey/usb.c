#include "types.h"
#include "memorymap.h"
#include "usb.h"

// private function declarations

// ----------------
// SIE low level (command building blocks)
// ----------------

/** low level SIE out phase (command or write)
	@param cmdCode value for cmdcode register
*/
void USB_SIE_Out(uint32_t cmdCode);

/** low level SIE in phase (read)
	@param cmdCode value for cmdcode register
	@return read value
*/
uint8_t USB_SIE_In(uint32_t cmdCode);

// ----------------
// SIE mid level (generic commands, using low level)
// ----------------

/** Command without writing or reading anything
	@param cmd command to send
*/
void USB_SIE_Command(USB_SIE_CommandID cmd);

/** Command with 1 byte write
	@param cmd command to send
	@param value value to send
*/
void USB_SIE_Command_Write1(USB_SIE_CommandID cmd, uint8_t value);

/** Command with 1 byte read
	@param cmd command to send
	@return read value
*/
uint8_t USB_SIE_Command_Read1(USB_SIE_CommandID cmd);

/** Command with 2 byte read
	@param cmd command to send
	@return read value (first low, second high)
*/
uint16_t USB_SIE_Command_Read2(USB_SIE_CommandID cmd);

typedef enum USB_SIE_CommandID {
	USB_SIE_SetAddress = 0xd0,				//device - write 1 byte
	USB_SIE_ConfigureDevice = 0xd8,				//device - write 1 byte
	USB_SIE_SetMode = 0xf3,					//device - write 1 byte
	USB_SIE_ReadInterruptStatus = 0xf4,			//device - read 1 or 2 bytes
	USB_SIE_ReadCurrentFrameNumber = 0xf5,			//device - read 1 or 2 bytes
	USB_SIE_ReadChipID = 0xfd,				//device - read 2 bytes
	USB_SIE_GetSetDeviceStatus = 0xfe,			//device - write 1 byte (set) or read 1 byte (get)
	USB_SIE_GetErrorCode = 0xff,				//device - read 1 byte
	USB_SIE_SelectEndpoint0 = 0x00,				//endpoint 0 - read 1 byte (optional)
	USB_SIE_SelectEndpoint1 = 0x01,				//endpoint 1 - read 1 byte (optional)
	USB_SIE_SelectEndpoint2 = 0x02,				//endpoint 2 - read 1 byte (optional)
	USB_SIE_SelectEndpoint3 = 0x03,				//endpoint 3 - read 1 byte (optional)
	USB_SIE_SelectEndpoint4 = 0x04,				//endpoint 4 - read 1 byte (optional)
	USB_SIE_SelectEndpoint5 = 0x05,				//endpoint 5 - read 1 byte (optional)
	USB_SIE_SelectEndpoint6 = 0x06,				//endpoint 6 - read 1 byte (optional)
	USB_SIE_SelectEndpoint7 = 0x07,				//endpoint 7 - read 1 byte (optional)
	USB_SIE_SelectEndpoint8 = 0x08,				//endpoint 8 - read 1 byte (optional)
	USB_SIE_SelectEndpoint9 = 0x09,				//endpoint 9 - read 1 byte (optional)
	USB_SIE_SelectEndpointClearInterruptSetStatus0 = 0x40,	//endpoint 0 - read 1 byte
	USB_SIE_SelectEndpointClearInterruptSetStatus1 = 0x41,	//endpoint 1 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus2 = 0x42,	//endpoint 2 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus3 = 0x43,	//endpoint 3 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus4 = 0x44,	//endpoint 4 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus5 = 0x45,	//endpoint 5 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus6 = 0x46,	//endpoint 6 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus7 = 0x47,	//endpoint 7 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus8 = 0x48,	//endpoint 8 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_SelectEndpointClearInterruptSetStatus9 = 0x49,	//endpoint 9 - read 1 byte (select/clear int) or write 1 byte (set endpoint status)
	USB_SIE_ClearBuffer = 0xf2,				//selected endpoint - read 1 byte (optional)
	USB_SIE_ValidateBuffer = 0xfa				//selected endpoint - no read/write
} USB_SIE_CommandID;

// ----------------
// SIE high level (real commands, use these)
// ----------------

/** sets the soft connect state
	@param connected true to make the device visible to the host, false otherwise
*/
void USB_SIE_SetConnected(bool connected);

/** returns the connected state 
	@return true if the device is connected (VBus sense), false otherwise
*/
bool USB_SIE_GetConnected(void);

/** sets the device address and enabled state.
	@param address 0..63
	@param enabled true if the device should respond to packets, false otherwise
*/
void USB_SIE_SetAddress(uint8_t address, bool enabled);

/** sets the device configured state
	@param configured true to set the device in the configured state, false for unconfigured
*/
void USB_SIE_ConfigureDevice(bool configured);

/** reads the SIE interrupt status
	@return bits 0-4 (0x1f): EP0-4 interrupt, bit 10 (0x0400): Device Status change interrupt
*/
uint16_t USB_SIE_ReadInterruptStatus();

// ----------------
// interrupt handlers
// ----------------

/** fast interrupt handler */
void usb_fiq_handler(void);

/** regular interrupt handler */
void usb_irq_handler(void);

// ----------------
// private implementation
// ----------------

void USB_SIE_Out(uint32_t cmdCode) {
	USB->DEVINTCLR =  USB_DEVINT_CC_EMPTY;			//clear cc empty interrupt flag
	USB->CMDCODE = cmdCode;					//send command
	while (!(USB->DEVINTST & USB_DEVINT_CC_EMPTY)) {}	//wait until cc is empty (command handled)
}

uint8_t USB_SIE_In(uint32_t cmdCode) {
	USB->DEVINTCLR =  USB_DEVINT_CC_EMPTY | USB_DEVINT_CD_FULL;	//clear cc empty and cd full interrupt flags
	USB->CMDCODE = cmdCode;						//send command
	while (!(USB->DEVINTST & USB_DEVINT_CD_FULL)) {}		//wait for cd full flag (data arrived)
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
	USB_SIE_Command_Write1(USB_SIE_GetSetDeviceStatus, connected ? 1 : 0);
}

bool USB_SIE_GetConnected(void) {
	return (USB_SIE_Command_Read1(USB_SIE_GetSetDeviceStatus) & 1) ? true : false;
}

void USB_SIE_SetAddress(uint8_t address, bool enabled) {
	USB_SIE_Command_Write1(USB_SIE_SetAddress, address + (enabled ? 1 : 0));
}

void USB_SIE_ConfigureDevice(bool configured) {
	USB_SIE_Command_Write1(USB_SIE_ConfigureDevice, configured ? 1 : 0);
}

uint16_t USB_SIE_ReadInterruptStatus() {
	return USB_SIE_Command_Read2(USB_SIE_ReaInterruptStatus);
}

void usb_fiq_handler(void) {
}

void usb_irq_handler(void) {
}

// ----------------
// public implementation
// ----------------

void USB_Init(void) {
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
	
	USB->DEVINTCLR = 0x3fff;			//clear all interrupts
	USB->DEVINTEN = 0x3fff;				//enable all interrupts
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

