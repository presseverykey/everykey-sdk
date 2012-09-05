#ifndef _USBSIE_
#define _USBSIE_

/** Functions for accessing the LPC1343's USB Serial Interface Engine. The SIE actually handles USB data communication. */

// ---------------------------------------
// SIE low level (command building blocks)
// ---------------------------------------

/** low level SIE out phase (command or write)
 @param cmdCode value for cmdcode register
 */
void USB_SIE_Out(uint32_t cmdCode);

/** low level SIE in phase (read)
 @param cmdCode value for cmdcode register
 @return read value
 */
uint8_t USB_SIE_In(uint32_t cmdCode);

// -------------------------------------------------
// SIE mid level (generic commands, using low level)
// -------------------------------------------------


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

// -----------------------------------------
// SIE high level (real commands, use these)
// -----------------------------------------

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

/** selects an endpoint and clears the interrupt
 @param epIdx physical endpoint index (0..9)
 @return an info bitfield about the endpoint status (USB_SELECT_EP_STATUS)
 */
uint8_t USB_SIE_SelectEndpointClearInterrupt(uint8_t epIdx);

/** sets an endpoint status
 @param epIdx physical endpoint index (0..9)
 @param status status bitfield (USB_SET_EP_STATUS)
 */
void USB_SIE_SetEndpointStatus(uint8_t epIdx, uint8_t status);


/** marks an OUT (device view: read) endpoint buffer as free. Will do nothing on isochronous endpoints.
 @param epIdx physical endpoint index (0..9)
 */
void USB_SIE_ClearBuffer(uint8_t epIdx);

/** marks an IN (device view: write) endpoint buffer as filled. Will do nothing on isochronous endpoints.
 @param epIdx physical endpoint index (0..9)
 */
void USB_SIE_ValidateBuffer(uint8_t epIdx);


#endif
