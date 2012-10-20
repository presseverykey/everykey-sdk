/** Anykey0x USB stack - USB Audio behaviour interface */

#ifndef _USBAUDIO_
#define _USBAUDIO_

#include "usb.h"
#include "usbaudiospec.h"

/** forward declaration of audio behaviour struct */
typedef struct _USBAudio_Behaviour_Struct USBAudio_Behaviour_Struct;

/** called when the host wants to write a control value. 
 @param device the USB device that originates this call
 @param behaviour the USB audio behaviour that originates this call
 @param request indicates which type is to be set. Usually only SET_CUR.
 @param nodeId node containing the control to set
 @param channelIdx index of channel to set
 @param selector selector of control to set
 @param channelId affected channel or 0 for master
 @param paramBlock pointer to buffer containing the value to be set
 @param paramBlockLength length of param block
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_SetControlValue)(USB_Device_Struct* device,
										 const USBAudio_Behaviour_Struct* behaviour,
										 USB_AUDIO_REQUEST request,
										 uint8_t nodeId,
										 uint8_t channelId,
										 USB_AUDIO_CONTROL_SELECTOR selector,
										 uint8_t* paramBlock,
										 uint8_t paramBlockLength);

/** called when the host wants to read a control value. 
 @param device the USB device that originates this call
 @param behaviour the USB audio behaviour that originates this call
 @param request indicates which type is to be read. GET_CUR, GET_MIN, GET_MAX or GET_RES.
 @param nodeId node containing the control to be read
 @param channelIdx index of channel to read
 @param selector selector of control to be read
 @param channelId affected channel or 0 for master
 @param paramBlock pointer to buffer to be filled with return value
 @param paramBlockLength length to fill
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_GetControlValue)(USB_Device_Struct* device,		
										 const USBAudio_Behaviour_Struct* behaviour,
										 USB_AUDIO_REQUEST request,
										 uint8_t nodeId,
										 uint8_t channelId,
										 USB_AUDIO_CONTROL_SELECTOR selector,
										 uint8_t* paramBlock,
										 uint8_t paramBlockLength);

/** called when the host wants to write an endpoint value. 
 @param device the USB device that originates this call
 @param behaviour the USB audio behaviour that originates this call
 @param request indicates which type is to be set. Usually only SET_CUR.
 @param endpoint the endpoint number (USB logical)
 @param selector selector of control to be set
 @param paramBlock pointer to buffer containing the value to be set
 @param paramBlockLength length of param block
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_SetEndpointValue)(USB_Device_Struct* device,
										  const USBAudio_Behaviour_Struct* behaviour,
										  USB_AUDIO_REQUEST request,
										  uint8_t endpoint,
										  USB_AUDIO_CONTROL_SELECTOR selector,
										  uint8_t* paramBlock,
										  uint8_t paramBlockLength);

/** called when the host wants to read an endpoint value. 
 @param device the USB device that originates this call
 @param behaviour the USB audio behaviour that originates this call
 @param request indicates which type is to be read. GET_CUR, GET_MIN, GET_MAX or GET_RES.
 @param endpoint the endpoint number (USB logical)
 @param selector selector of control to be read
 @param paramBlock pointer to buffer to be filled with return value
 @param paramBlockLength length to fill
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_GetEndpointValue)(USB_Device_Struct* device,
										  const USBAudio_Behaviour_Struct* behaviour,
										  USB_AUDIO_REQUEST request,
										  uint8_t endpoint,
										  USB_AUDIO_CONTROL_SELECTOR selector,
										  uint8_t* paramBlock,
										  uint8_t paramBlockLength);


/** called when one of the audio streaming interface changes alt setting.
 This call may be used to intercept invalid alt settings or to observe the 
 streaming state of the interfaces. 
 @param device the USB device that originates this call
 @param behaviour the USB audio behaviour that originates this call
 @param interface the interface number that is requested to change
 @param altValue the new alt setting
 @return true on success, false otherwise */
 typedef bool (*USBAudio_InterfaceAltChangeCallback)(USB_Device_Struct* device,
													 const USBAudio_Behaviour_Struct* behaviour,				
													 uint8_t interface,
													 uint8_t altValue);

/** called on a USB configuration change. May be used for setting up the behaviour's state.
 @param device the USB device that originates this call
 @param behaviour the USB audio behaviour that originates this call
 @param configuration the new configuration value */
typedef void (*USBAudio_ConfigChangeCallback)(USB_Device_Struct* device,
											  const USBAudio_Behaviour_Struct* behaviour);

/** called each USB frame. Implementors should read and/or write their audio data.
 Called each USB frame, independent of streaming states. If CPU-intense tasks are carried out,
 streaming state should be checked (either by implementing the 
 USBAudio_SetStreamingInterfaceAltSetting callback and remembering the state or by 
 querying the USB device's current interface alt setting.
 @param device the USB device that originates this call
 @param behaviour the USB audio behaviour that originates this call */
typedef void (*USBAudio_FrameCallback)(USB_Device_Struct* device,
									   const USBAudio_Behaviour_Struct* behaviour);


/** Our audio device behaviour struct. Contents may change, must be in RAM. */
struct _USBAudio_Behaviour_Struct {
	/** all custom behaviours must start with a base behaviour - pointers will be typecast */
	USB_Behaviour_Struct baseBehaviour;			

	/** callback for unit value get requests. If NULL, all requests will result in a stall. */
	USBAudio_GetControlValue getControlValueCallback;

	/** callback for unit value set requests. If NULL, all requests will result in a stall. */
	USBAudio_SetControlValue setControlValueCallback;

	/** callback for endpoint value get requests. If NULL, all requests will result in a stall. */
	USBAudio_GetEndpointValue getEndpointValueCallback;
	
	/** callback for endpoint value set requests. If NULL, all requests will result in a stall. */
	USBAudio_SetEndpointValue setEndpointValueCallback;

	/** callback for interface alt setting changes. If NULL, all requests will silently be accepted. */
	USBAudio_InterfaceAltChangeCallback altChangeCallback;

	/** callback for configutation changes. If NULL, all requests will silently be accepted. */
	USBAudio_ConfigChangeCallback configChangeCallback;
	
	/** USB frame callback for actual streaming. If NULL, nothing will be streamed. */
	USBAudio_FrameCallback frameCallback;
	
	/** the interface number of the control interface */
	uint8_t controlInterface;

	/** the interface number of the in (device-to-host) stream interface - set to 0xff if unused.
	 USB Audio implementation is currently limited to one in stream. LPC1343 only has one in iscoch endpoint anyway. */
	uint8_t inStreamInterface;

	/** the interface number of the out (host-to-device) stream interface - set to 0xff if unused.
	 USB Audio implementation is currently limited to one out stream. LPC1343 only has one out iscoch endpoint anyway. */
	uint8_t outStreamInterface;

	/** the logical endpoint of the audio in stream (always 0x84 for LPC1343, if used). set to 0xff if unused. */
	uint8_t inStreamEndpoint;

	/** the logical endpoint of the audio out stream (always 0x04 for LPC1343, if used). set to 0xff if unused. */
	uint8_t outStreamEndpoint;

};

/** USB Audio base behaviour handlers. May be used for initalizing a USBAudio_Behaviour_Struct
 at runtime. If you want to initialize a const USBAudio_Behaviour_Struct at compile,
 you may use the MAKE_USBAUDIO_BASE_BEHAVIOUR macro. */
bool USBAudio_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

void USBAudio_FrameHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

bool USBAudio_InterfaceAltHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t interface, uint8_t newAlt);

void USBAudio_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

#define MAKE_USBAUDIO_BASE_BEHAVIOUR {\
	USBAudio_ExtendedControlSetupHandler, \
	NULL, /* we only have isoch endpoints - isoch endpoints are not notified */\
	USBAudio_FrameHandler,\
	USBAudio_InterfaceAltHandler,\
	USBAudio_ConfigChangeHandler\
}


#endif
