/** Anykey0x USB stack CDC class implementation. Note this is not a complete,
 generic CDC implementation due to the diversity of CDC.
 
 Currently, it only supports Abstract Control Model in a rather standard
 flavor to provice simple cross-platform host communication through a virtual
 serial port. */

#ifndef _USBCDC_
#define _USBCDC_

#include "../pressanykey/types.h"
#include "cdcspec.h"
#include "usb.h"
#include "ringbuffer.h"

#pragma mark CDC-specific callbacks

//CDC behaviour forward typedef
typedef struct _USBCDC_Behaviour_Struct USBCDC_Behaviour_Struct;

//CDC callbacks go here

/** called when the host sent a break
 @param device the usb device
 @param behaviour the cdc behaviour
 @param durationMS duration in milliseconds */
typedef void (*USBCDC_BreakCallback)(USB_Device_Struct* device,
									const USBCDC_Behaviour_Struct* behaviour,
									uint16_t durationMS); 

/** called when the host wants to change linecoding settings
 @param device the usb device
 @param behaviour the cdc behaviour
 @return true to accept the change (currentLineCoding will be changed automatically)
 or false to reject.*/
typedef bool (*USBCDC_LineCodingChangeCallback)(USB_Device_Struct* device,
											   const USBCDC_Behaviour_Struct* behaviour,
											   const USB_CDC_Linecoding_Struct* newLineCoding); 

/** called when the host changed the idle state via SetCommFeature or ClearCommFeature
 @param device the usb device
 @param behaviour the cdc behaviour */
typedef bool (*USBCDC_IdleChangeCallback)(USB_Device_Struct* device,
										 const USBCDC_Behaviour_Struct* behaviour); 

/** called when the host changed the control line state via SetControlLineState
 @param device the usb device
 @param behaviour the cdc behaviour */
typedef bool (*USBCDC_ControlLineChangeCallback)(USB_Device_Struct* device,
										 const USBCDC_Behaviour_Struct* behaviour); 

/** called when data was received from host. Note that this (and other callbacks) are
 issued at interrupt time - should be kept short.
 @param device the usb device
 @param behaviour the cdc behaviour */
typedef bool (*USBCDC_DataAvailableCallback)(USB_Device_Struct* device,
										USBCDC_Behaviour_Struct* behaviour); 


#pragma mark Structs

/** description of a CDC class behaviour. Runtime state is only referenced,
 not included, so that this structure may go into Flash. Make sure you point
 to some valid RAM space in the respective pointers */
struct _USBCDC_Behaviour_Struct {
	
	/** must be first in behaviour implementations */
	USB_Behaviour_Struct baseBehaviour;
	
	/** set a callback to listen to breaks sent from the host. Set to null to ignore. */
	USBCDC_BreakCallback breakCallback;
	
	/** set a callback to intercept / check line coding changes. Set to null to accept all changes */
	USBCDC_LineCodingChangeCallback lineCodingChangeCallback;
	
	/** set a callback to be informed about idle changes, set to null to ignore */
	USBCDC_IdleChangeCallback idleChangeCallback;

	/** set a callback to be informed about control line changes, set to null to ignore */
	USBCDC_ControlLineChangeCallback controlLineChangeCallback;

	/** set to be informed about available data. Called from interrupt, so keep it short. set to null to ignore */
	USBCDC_DataAvailableCallback dataAvailableCallback;

	/** reset value of our line coding. */
	USB_CDC_Linecoding_Struct defaultLineCoding;
	
	/** our current line coding mode (speed, data bits, parity, stop bits).
	 Point to a valid and initialized struct in RAM. */
	USB_CDC_Linecoding_Struct* currentLineCoding;

	/** ring buffer for receiving data from host */
	RingBufferStatic hostToDeviceBuffer;

	/** ring buffer for sending data to host */
	RingBufferStatic deviceToHostBuffer;

	/** idle state (CommFeature->Abstract_state->idle), point to valid cleared bool in RAM */
	bool* idle;
	
	/** control line state (via SetControlLineState), point to valid cleared uint8_t in RAM */
	uint8_t* controlLineState;

	/** interface number of the control interface */
	uint8_t controlInterface;

	/** interface number of the data interface */
	uint8_t dataInterface;
	
	/** physical endpoint number of the data in (device to host) endpoint */
	uint8_t dataInEndpoint;

	/** physical endpoint number of the data out (host to device) endpoint */
	uint8_t dataOutEndpoint;

	/** physical endpoint number of the in (device to host) interrupt */
	uint8_t interruptEndpoint;
	
};

#pragma mark CDC class USB behaviour callbacks

/** you may use these to manually initialize a CDC behaviour at runtime, for
 compile-time assembly, you can use the MAKE_USBCDC_BASE_BEHAVIOUR macro. */

bool USBCDC_ExtendedControlSetupHandler(USB_Device_Struct* device,
										const USB_Behaviour_Struct* behaviour);

bool USBCDC_EndpointDataHandler(USB_Device_Struct* device,
								const USB_Behaviour_Struct* behaviour, uint8_t epIdx);

void USBCDC_ConfigChangeHandler(USB_Device_Struct* device,
								const USB_Behaviour_Struct* behaviour);


#define MAKE_USBCDC_BASE_BEHAVIOUR {\
	USBCDC_ExtendedControlSetupHandler,\
	USBCDC_EndpointDataHandler,\
	NULL,\
	NULL,\
	USBCDC_ConfigChangeHandler\
}


#pragma mark CDC functions for user code


/** resets the behaviour to default values
 @param behaviour CDC behaviour */ 
void USBCDC_ResetBehaviour(const USBCDC_Behaviour_Struct* cdc);


/** notifies the host that the (virtual) serial end was connected or disconnected 
 @param device the USB device
 @param behaviour the CDC behaviour
 @param connected true if a serial device is connected, false if not */
void USBCDC_SendNetworkConnectionChange(USB_Device_Struct* device,
										const USB_Behaviour_Struct* behaviour,
										bool connected);


/** reads serial data to host.
 @param device the USB device
 @param cdc the CDC behaviour
 @param buffer buffer to fill
 @param maxLen max length to read
 @return number of bytes actually read */
uint16_t USBCDC_ReadBytes(USB_Device_Struct* device,
						  const USBCDC_Behaviour_Struct* cdc,
						  uint8_t* buffer, uint16_t maxLen);
	

/** writes serial data to host.
 @param device the USB device
 @param cdc the CDC behaviour
 @param buffer buffer containing data to write
 @param maxLen max length to write
 @return number of bytes actually written */
uint16_t USBCDC_WriteBytes(USB_Device_Struct* device,
						   const USBCDC_Behaviour_Struct* cdc,
						   uint8_t* buffer, uint16_t maxLen);
	

#endif