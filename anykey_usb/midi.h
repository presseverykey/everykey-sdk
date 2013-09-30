#ifndef _USBMIDI_
#define _USBMIDI_

#include "usb.h"

/** A simple MIDI class implementation, slightly limited */

//forward typedef of USBMIDI_Behaviour_Struct
typedef struct _USBMIDI_Behaviour_Struct USBMIDI_Behaviour_Struct;

/** called when device-to-host data can be sent. Implementations should
 * fill outBuffer with current values.
 * @param device USB device this call originated from
 * @param behaviour the behaviour this call originated from
 * @return length of valid data */

typedef uint16_t (*MidiDataInHandler)( USB_Device_Struct* device,
                                       const USBMIDI_Behaviour_Struct* behaviour);


/** called when a host-to-device data has arrived in inBuffer
 * @param device USB device this call originated from
 * @param behaviour the behaviour this call originated from
 * @param endpoint originating OUT endpoint (for interfaces with multiple bulk data OUT endpoints)
 * @param len langth of data */

typedef void (*MidiDataOutHandler)( USB_Device_Struct* device,
									const USBMIDI_Behaviour_Struct* behaviour,
									uint16_t len);


/** our MIDI behaviour representation - state variables are added as
 *  pointers to RAM, so this struct may be const */

struct _USBMIDI_Behaviour_Struct {
	USB_Behaviour_Struct baseBehaviour;
	uint8_t interfaceNumber;
	uint8_t* inBuffer;                  //Set to null for unused
	uint8_t* outBuffer;                 //Set to null for unused
	uint8_t inDataEndpoint;
	uint8_t outDataEndpoint;
	MidiDataInHandler inDataHandler;
	MidiDataOutHandler outDataHandler;
};

/* USBHID base behaviour handlers. May be used to manually initialize a
 * USB_Behaviour_Struct at runtime,
 * for compile time initialization, you may use the
 * MAKE_USBHID_BASE_BEHAVIOUR macro. */

bool USBMIDI_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

bool USBMIDI_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx);

void USBMIDI_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

#define MAKE_USBMIDI_BASE_BEHAVIOUR {\
	USBMIDI_ExtendedControlSetupHandler,\
	USBMIDI_EndpointDataHandler,\
	NULL,\
	NULL,\
	USBMIDI_ConfigChangeHandler\
}



#endif
