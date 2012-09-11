#ifndef _USB_
#define _USB_

#include "../pressanykey/types.h"
#include "usbspec.h"

/** General notes: The terminology of this USB stack tries to adhere to the USB specification.
 * This helps reading the code and the spec, but may be confusing in the beginning.
 * For example, directions always relate to the host: IN means device-to-host,
 * OUT means host-to-device.
 *
 * Unless otherwise noted, endpoints are addressed by physical endpoints on the device.
 * Physical epIdx: bit 0: direction (0 = OUT, 1 = IN)
 * Physical epIdx: bits 1..4: logical endpoint number
 * USB spec endpoint address: bits 0..3: logical endpoint number
 * USB spec endpoint address: bit 7: direction (0 = OUT, 1 = IN)
 */

/** change limits if you need more */
#define USB_MAX_CONFIGURATION_COUNT 3
#define USB_MAX_STRING_COUNT 5

/** fixed by spec and device */
#define USB_MAX_COMMAND_PACKET_SIZE 64

#pragma mark USB Types

/** Set extendedControlSetupCallback when you want to respond to class- or
 * device-specific commands (setup packets on the control out pipe). Otherwise, set to NULL.
 * The function is called upon arrival of a new control setup block, before the
 * USB driver handles standard requests, so that the callback can override standard behaviour.
 * Implementations should check the current setup packet in the global variable usbCurrentCommand.
 * If the callback handles the command, it should return true, false otherwise. If data is to be
 * transferred within the command's data phase, the callback should set the global
 * usbCurrentCommandDataBase to the base of the send / receive buffer (determined by the
 * direction bit in usbCurrentCommand.bmRequestType) and the number of bytes to transfer into
 * usbCurrentCommandBytesRemaining. Additionally, it may set usbControlOutDataCompleteCallback or/and
 * usbControlStatusCallback to be notified when the data or status phase of the command has completed. */
typedef	bool (*ExtendedControlSetupCallback)();

/** Set endpointDataCallback to handle data transfer on non-control endpoints.
 * The physical endpoint index is passed as parameter. Out (host-to-device) endpoints
 * have even endpoint indexes, the callback is called when a transfer has finished,
 * which means that the buffer is free to be filled again via USB_EP_Write(). When
 * the buffer is not filled, read requests by the host are silently NAK-ed.
 * In (device-to-host) endpoints have uneven indexes. For them, the callback is called
 * when data has arrived, ready to be read via USB_EP_Read(). */
typedef	void (*EndpointDataCallback)(uint8_t epIdx);

/**init structure describing the device to be implemented */
typedef struct USB_Device_Struct {
	/** main device descriptor. Length is taken from [0]. Must not be NULL. */
	const uint8_t* deviceDescriptor;

	/** number of valid configurations */
	uint8_t configurationCount;

	/** configuration descriptors, including interfaces and endpoints.
	 * Entries 0..configurationCount-1 must not be NULL. Length is taken from [2] and [3] */
	const uint8_t* configurationDescriptors[USB_MAX_CONFIGURATION_COUNT];

	/** number of valid string descriptors, including language string descriptor */
	uint8_t stringCount;

	/** all string descriptors, including language string descriptor.
	 * Entries 0..stringCount-1 must not be NULL. Length is taken from [0]. */
	const uint8_t* strings[USB_MAX_STRING_COUNT];

	/** callback for handling device-specific USB commands */
	ExtendedControlSetupCallback extendedControlSetupCallback;

	/** callback for handling data transfers */
	EndpointDataCallback endpointDataCallback;
} USB_Device_Struct;

#pragma mark USB Functions

/** reads from an endpoint
 @param epIdx physical endpoint index (must be OUT)
 @param buffer read buffer (must not be null if length > 0). Reads will be multiples of 4, make buffer large enough.
 @param length max length to read. Rest of packet will be skipped.
 @return number of bytes actually read */
uint32_t USB_EP_Read(uint8_t epIdx, uint8_t* buffer, uint32_t length);

/** writes to an endpoint
 @param epIdx physical endpoint index (must be IN)
 @param buffer write buffer (must not be null if length > 0)
 @param length max length to write
 @return number of bytes actually written */
uint32_t USB_EP_Write(uint8_t epIdx, const uint8_t* buffer, uint32_t length);

/** stalls or unstalls a given endpoint.
	@param epIdx physical endpoint index
	@param stalled true to stall, false to unstall */
void USB_EP_SetStall(uint8_t epIdx, bool stalled);

/** returns the stall state of an endpoint
	@param epIdx physical endpoint index
	@return the corresponding native index - must still be checked for validity */
bool USB_EP_GetStall(uint8_t epIdx);

/** powers up required blocks, sets up clock etc. Leaves soft-connect disconnected.
 * @param deviceDefinition Pointer to correctly and completely filled USB_Device_Struct. Must not be NULL.
 */
void USB_Init(USB_Device_Struct* deviceDefinition);

/** enable connection from client side (soft-connect) */
void USB_SoftConnect(void);

/** disable connection from client side (soft-connect) */
void USB_SoftDisconnect(void);

/** returns whether the device is connected (VBUS present) */
bool USB_Connected(void);

/** converts usb endpoint indexes (dir at bit 7) to native indexes (dir at bit 0)
 @param index usb endpoint index
 @return physical endpoint index */
uint8_t USB_EP_LogicalToPhysicalIndex(uint8_t index);

#pragma mark USB global variables

/** current USB command */
extern USB_Setup_Packet usbCurrentCommand;

/** current command data phase data. Init in setup handler if needed */
extern uint8_t* usbCurrentCommandDataBase;	//handled like const if current command is USB_RT_DIR_DEVICE_TO_HOST

/** current command data phase length. Init in setup handler if needed */
extern uint32_t usbCurrentCommandDataRemaining;

/** current command data phase completed callback. Init in setup handler if needed */
extern void (*usbControlOutDataCompleteCallback)(void);		//callback when host-to-device has arrived

/** current command status phase completed callback. Init in setup handler if needed */
extern void (*usbControlStatusCallback)(void);				//callback when status was sent



#endif
