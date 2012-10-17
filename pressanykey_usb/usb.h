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
#define USB_MAX_INTERFACES_PER_DEVICE 8
#define USB_MAX_BEHAVIOURS_PER_DEVICE 4

#define USB_MAX_COMMAND_PACKET_SIZE 64
#define USB_MAX_COMMAND_DATA_SIZE 64

#pragma mark Function prototypes

// forward declarations of structs
typedef struct USB_Device_Struct USB_Device_Struct;
typedef struct USB_Behaviour_Struct USB_Behaviour_Struct;

/** Set `extendedControlSetupCallback` when you want to respond to class-
 *  or device-specific commands (setup packets on the CONTROL OUT pipe).
 *  Otherwise, set to NULL.  The function is called upon arrival of a new
 *  control setup block, before the USB driver handles standard requests,
 *  so that the callback can override standard behaviour.
 *  Implementations should check the current setup packet in the device
 *  struct's `currentCommand`.  If the callback handles the command, it
 *  should return `true`, `false` otherwise. If data is to be transferred
 *  within the command's data phase, the callback should set the device's
 *  `currentCommandDataBase` to the base of the send / receive buffer
 *  (determined by the direction bit in `currentCommand.bmRequestType`) and
 *  the number of bytes to transfer into `currentCommandBytesRemaining`.
 *  Additionally, it may set `controlOutDataCompleteCallback` or/and
 *  `controlStatusCallback` to be notified when the data or status phase of
 *  the command has completed. */

typedef	bool (*USBExtendedControlSetupCallback)(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);


/** Set `endpointDataCallback` to handle data transfer on non-control
 *  endpoints.  The physical endpoint index is passed as parameter. OUT
 *  (host-to-device) endpoints have even endpoint indexes, the callback
 *  is called when a transfer has finished, which means that the buffer
 *  is free to be filled again via `USB_EP_Write()`. When the buffer is not
 *  filled, read requests by the host are silently NAK-ed.  In
 *  (device-to-host) endpoints have uneven indexes. For them, the
 *  callback is called when data has arrived, ready to be read via
 *  `USB_EP_Read()`. */

typedef	bool (*USBEndpointDataCallback)(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx);


/** Set `frameCallback` to be notified on every USB frame. This callback
 *  is mainly useful for isochronous endpoints - data flow on isochronous
 *  pipes is not notified by `endpointDataCallback`.  Instead, they must be
 *  read and written each frame. */

typedef void (*USBFrameCallback)(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);


/** Set `interfaceAltCallback` to be notified on changes of interface alt
 *  settings. This callback is useful for changing alt-setting-dependent
 *  functionality such as starting or stopping isochronous streaming.
 *  Return true if the new alt setting is ok, false otherwise.  If
 *  unused, alt changes will fail. */

typedef bool (*USBInterfaceAltCallback)(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t interface, uint8_t newAlt);


/** Set `configChangeCallback` to be notified about USB device
 *  configuration changes. This callback can be used for setting up
 *  internal state variables. The new config value can be read from the
 *  device struct */

typedef void (*USBConfigChangeCallback)(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);


/** temporary callback for setup packet handlers - used in order to be called
 *  back after the data stage of an OUT CONTROL transfer */

typedef bool (*USBControlOutDataCompleteCallback)(USB_Device_Struct* device);	


/** temporary callback for setup packet handlers - used in order to be called
 * back after the status stage of a control transfer */

typedef bool (*USBControlStatusCallback)(USB_Device_Struct* device);


#pragma mark Structures


/** This structure defines callbacks for class- or vendor-specific
 *  functionality. One or multiple behaviours can be attached to a USB
 *  device. Typical behaviour implementations create a new struct that
 *  starts with this substructure and continues with their own entries so
 *  that pointers can be typecast back and forth (poor man's
 *  subclassing). If behaviours want to keep state information, they
 *  should do so using pointers to RAM so that this struct (and its
 *  wrappers) may be const to keep them in Flash. */

typedef struct USB_Behaviour_Struct {

	/** callback for handling device-specific USB commands - setting to
	 * NULL will not accept any control requests.  The request is offered
	 * to all behaviours until one handles it.  If none of them returns
	 * `true`, standard USB handlers attempt to handle it.  If that fails,
	 * the OUT pipe will be stalled.  */

	USBExtendedControlSetupCallback extendedControlSetupCallback;


	/** callback for handling data transfers - called either when OUT data
	 *  is available or when prior queued IN data was sent. Behaviours
	 *  responsible for a non-control endpoint should implement this
	 *  method, trigger other activities if necessary and return `true`.
	 *  Setting to NULL will act as always returning `false`. The message
	 *  is passed to all registered behaviours until one of them returns
	 *  `true`.  If none of them handles this message, the endpoint will
	 *  stall. */

	USBEndpointDataCallback endpointDataCallback;


	/** callback invoked each USB frame - set to NULL if not used */

	USBFrameCallback frameCallback;


	/** callback invoked on interface alt changes - setting to NULL will
	 *  not accept any alt setting change for this behaviour */

	USBInterfaceAltCallback interfaceAltCallback;


	/** callback invoked on device config changes - set to NULL if not
	 *  used */

	USBConfigChangeCallback configChangeCallback;
	
} USB_Behaviour_Struct;


/** A structure describing the static properties of a USB device. May be
 * in RAM or Flash, if initialized at compile-time.  Must be initialized
 * before passing it to `USB_Init`. */

typedef struct USB_Device_Definition {

	/** main device descriptor. Length is taken from [0]. Must not be
	 *  NULL. Device descriptors are defined in usb_20.pdf Table 9-8
	 *  p.262(p.290 pdf) */
	const uint8_t* deviceDescriptor;


	/** number of valid configurations */
	uint8_t configurationCount;
	
	/** configuration descriptors, including interfaces and endpoints.
	 *  Entries 0..configurationCount-1 must not be NULL. Length is taken
	 *  from [2] and [3]. Configuration Descriptors are defined in
	 *  usb_20.pdf Table 9-10 p.265 (p.293 pdf)
	 */
	const uint8_t* configurationDescriptors[USB_MAX_CONFIGURATION_COUNT];
	
	/** number of valid string descriptors, including language string
	 * descriptor */
	uint8_t stringCount;
	
	/** all string descriptors, including language string descriptor.
	 * Entries 0..stringCount-1 must not be NULL. Length is taken from
	 * [0]. */
	const uint8_t* strings[USB_MAX_STRING_COUNT];
	
	/** number of currently attached behaviours */
	uint8_t behaviourCount;
	
	/** a list of attached behaviours */
	USB_Behaviour_Struct* behaviours[USB_MAX_BEHAVIOURS_PER_DEVICE];
	
} USB_Device_Definition;

/* A structure defining the runtime state of a USB device. Must be in
 * RAM. Can be uninitialized before passing it to `USB_Init`. */

typedef struct USB_Device_Struct {

	/** a reference to our static device definition */
	const USB_Device_Definition* deviceDefinition;
	
	/** callback when host-to-device has arrived - may be set by setup
	 *  packet handlers */
	USBControlOutDataCompleteCallback controlOutDataCompleteCallback;
	
	/** callback when status was sent - may be set by setup packet
	 *  handlers */
	USBControlStatusCallback controlStatusCallback;

	/** reference for per-command callbacks above, may be set by setup
	 *  packet handlers to be read back from callbacks */
	void* callbackRefcon;
	
	/** Data buffer for USB command OUT data phase */
	uint8_t commandDataBuffer[USB_MAX_COMMAND_DATA_SIZE];
	
	/** Remember our alt interface settings */
	uint8_t	interfaceAltSetting[USB_MAX_INTERFACES_PER_DEVICE];
	
	/** remember if we're suspended or not */
	bool usbSuspended;
	
	/** Current configuration */
	uint8_t currentConfiguration;
	
	/** Currently pending command */
	USB_Setup_Packet currentCommand;
	
	/** Pointer for command data phase - may point to const if dir is DEVICE_TO_HOST */
	uint8_t* currentCommandDataBase;			
	
	/** Number of remaining bytes to transfer in data phase */
	uint32_t currentCommandDataRemaining;
		
} USB_Device_Struct;

#pragma mark USB Functions

/** reads from an endpoint
 * @param device device to check 
 * @param epIdx physical endpoint index (must be OUT)
 * @param buffer read buffer (must not be null if length > 0). Reads
 *        will be multiples of 4, make buffer large enough.
 * @param length max length to read. Rest of packet will be skipped.
 * @return number of bytes actually read */
uint32_t USB_EP_Read(USB_Device_Struct* device, uint8_t epIdx, uint8_t* buffer, uint32_t length);

/** writes to an endpoint
 * @param device device to check 
 * @param epIdx physical endpoint index (must be IN)
 * @param buffer write buffer (must not be null if length > 0)
 * @param length max length to write
 * @return number of bytes actually written */
uint32_t USB_EP_Write(USB_Device_Struct* device, uint8_t epIdx, const uint8_t* buffer, uint32_t length);

/** stalls or unstalls a given endpoint
 * @param device device to modify 
 * @param epIdx physical endpoint  
 * @param stalled true to stall, false to unstall */
void USB_EP_SetStall(USB_Device_Struct* device, uint8_t epIdx, bool stalled);

/** returns the stall state of an endpoint
 * @param device device to check 
 * @param epIdx physical endpoint index
 * @return the corresponding native index - must still be checked for validity */
bool USB_EP_GetStall(USB_Device_Struct* device, uint8_t epIdx);

/** powers up required blocks, sets up clock etc. Leaves soft-connect disconnected.
 * If used before, clears out all runtime state and previously attached behaviours.
 * @param definition Pointer to correctly filled USB_Device_Definition. Must not be NULL.
 * @param device Pointer to uninitialized USB_Device_Struct. Memory must
 *        be kept during existence of USB device. Must not be NULL.
 */
void USB_Init(const USB_Device_Definition* definition,
USB_Device_Struct* device);

/** enable connection from client side (soft-connect) 
	* @param device device to connect */
void USB_SoftConnect(USB_Device_Struct* device);

/** disable connection from client side (soft-connect)
	* @param device device to disconnect */
void USB_SoftDisconnect(USB_Device_Struct* device);

/** returns whether the device is connected (VBUS present) 
 * @param device device to check */
bool USB_Connected(USB_Device_Struct* device);

/** converts usb endpoint indexes (dir at bit 7) to native indexes (dir at bit 0)
 @param index usb endpoint index
 @return physical endpoint index */
uint8_t USB_EP_LogicalToPhysicalIndex(uint8_t index);

#endif
