#ifndef _USBMIDI_
#define _USBMIDI_

#include "usb.h"

/** A simple MIDI class implementation, slightly limited */

//forward typedef of USBMIDI_Behaviour_Struct
typedef struct _USBMIDI_Behaviour_Struct USBMIDI_Behaviour_Struct;

/** these handlers are invoked when MIDI data has arrived. Note that these are called from interrupts, so keep it short. */
typedef void (*MidiNoteOnHandler)(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t note, uint8_t velocity);
typedef void (*MidiNoteOffHandler)(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t note);
typedef void (*MidiControlChangeHandler)(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t control, uint8_t value);
typedef void (*MidiPitchBendChangeHandler)(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint16_t bend);
typedef void (*MidiSysExHandler)(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t* data, uint8_t length, bool moreToCome);

/** Use these functions to send MIDI data to the host. Return true for success, false otherwise. */
bool USBMIDI_SendNoteOn(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t note, uint8_t velocity);
bool USBMIDI_SendNoteOff(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t note);
bool USBMIDI_SendControlChange(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t control, uint8_t value);
bool USBMIDI_SendSysEx(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t* data, uint8_t len);

/** our MIDI behaviour representation - state variables are added as
 *  pointers to RAM, so this struct may be const */

struct _USBMIDI_Behaviour_Struct {
	USB_Behaviour_Struct baseBehaviour;             //Generic bahaviour settings, use MAKE_USBMIDI_BASE_BEHAVIOUR
	uint8_t interfaceNumber;                        //Interface number of MIDI behaviour
	uint8_t* inBuffer;                              //Size: USB_MAX_BULK_DATA_SIZE. Set to null for unused
	uint8_t* outBuffer;                             //Size: USB_MAX_BULK_DATA_SIZE. Set to null for unused
	uint8_t inDataEndpoint;                         //Index of bulk endpoint (physical) for IN data (device to host)
	uint8_t outDataEndpoint;                        //Index of bulk endpoint (physical) for OUT data (host to device)
	uint8_t* cmdFifo;                               //Pass a buffer for outbound messages. 
	uint16_t cmdFifoSize;                           //Size of cmdFifo - multiple of 4. More means more out data can be queued
	uint16_t* cmdFifoRdIdx;                         //Pointer to a uint16_t in RAM (init with 0)
	uint16_t* cmdFifoWrIdx;                         //Pointer to a uint16_t in RAM (init with 0)
	MidiNoteOnHandler noteOnHandler;                //Set to be informed about incoming NOTE ON events. Set to NULL to ignore.
	MidiNoteOffHandler noteOffHandler;              //Set to be informed about incoming NOTE OFF events. Set to NULL to ignore.
	MidiControlChangeHandler controlChangeHandler;  //Set to be informed about incoming CC events. Set to NULL to ignore.
	MidiPitchBendChangeHandler pitchBendHandler;	//Set to be informed about incoming pitch bend events. Set to NULL to ignore
	MidiSysExHandler sysExHandler;                  //Set to be informed about incoming SysEx events (data will be sent in chunks). Set to NULL to ignore.
};

/* USBHID base behaviour handlers. May be used to manually initialize a USB_Behaviour_Struct at runtime,
 * for compile time initialization, you may use the MAKE_USBHID_BASE_BEHAVIOUR macro. You usually should not need to use these directly. */

bool USBMIDI_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

bool USBMIDI_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx);

void USBMIDI_FrameHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

void USBMIDI_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

#define MAKE_USBMIDI_BASE_BEHAVIOUR {\
	USBMIDI_ExtendedControlSetupHandler,\
	USBMIDI_EndpointDataHandler,\
	USBMIDI_FrameHandler,\
	NULL,\
	USBMIDI_ConfigChangeHandler\
}



#endif
