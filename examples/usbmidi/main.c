#include "anykey/anykey.h"
#include "anypio.h"

// This example shows how to implement a USB MIDI device on the Anykey.
// Because USB is quite an elaborate topic, we've tryed to keep as much 
// of the USB specifics out of this tutorial to concentrate on the code
// you need to implement.

// In case you are interested in the "nitty gritty" details, refer to
// the header:

#include "usbmidiboilerplate.h"

// as well as the code contained in the linked "anykey_usb" directory.
// We'll get back to this code in a moment.

// In case you're really itching to just get going and don't care how
// things work, you can also consider the following code as boilerplate
// that you just need to copy and skip ahead to the comment below labeled:
//
//   /** MAIN PROGRAM FROM HERE */
// 

// First, we need to declare some storage in RAM for the USB
// implemtation to use for buffering MIDI data received from the host as
// well as MIDI data queued for sending to the host. 

// The size of the MIDI output FIFO needs to be a multiple of 4 bytes in
// length, apart from that, the sizes are more or less arbitrary.

#define MIDI_FIFO_SIZE 256
#define MAX_SYSEX_SIZE 128

uint8_t inBuffer[USB_MAX_BULK_DATA_SIZE];				//data from device to host
uint8_t outBuffer[USB_MAX_BULK_DATA_SIZE];			//data from host to device
uint8_t midiFifo[MIDI_FIFO_SIZE];
uint16_t midiFifoRdIdx = 0;
uint16_t midiFifoWrIdx = 0;

// Next we need to define our devices behaviour: 
// In order to send MIDI to the host, we use functions declared in:
//    anykey_usb/midi.h
//
// USBMIDI_SendNoteOn
// USBMIDI_SendNoteOff
// USBMIDI_SendControlChange
// USBMIDI_SendSysEx
//
// The MIDI data that is received from the host is passed to the
// following callbacks:

void myMidiNoteOnHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note, uint8_t velocity);
void myMidiNoteOffHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note);
void myMidiControlChangeHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t control, uint8_t value);
void myMidiSysExHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t* data, uint8_t length, bool moreToCome);

// The Behaviour contains all the information necessary to handle MIDI
// data once the connection to the host is established. 

const USBMIDI_Behaviour_Struct midi_behaviour = {
	MAKE_USBMIDI_BASE_BEHAVIOUR,
	0,
	inBuffer,
	outBuffer,
	DATA_IN_ENDPOINT_PHYSICAL,
	DATA_OUT_ENDPOINT_PHYSICAL,
	midiFifo,
	MIDI_FIFO_SIZE,
	&midiFifoRdIdx,
	&midiFifoWrIdx,
	myMidiNoteOnHandler,
	myMidiNoteOffHandler,
	myMidiControlChangeHandler,
	myMidiSysExHandler
};

// The 'usbmidiboilerplate.h' header file contains the necessary USB
// descriptors that the Anykey will send to the host to identify itself
// as a MIDI, as well as the behaviour described above. This struct is
// necessary to initialize the USB hardware.

const USB_Device_Definition midi_deviceDefinition = {
        midi_deviceDescriptor,
        1,
        { midi_configDescriptor },
        4,
        { midi_languages, midi_manufacturerName, midi_deviceName, midi_serialName },
        1,
        { (USB_Behaviour_Struct*)(&midi_behaviour) }
};

// The USB hardware (i.e. it's runtime state) on the anykey is
// contained in a `USB_Device_Struct`, which we declare here:

USB_Device_Struct midi_device;


/** MAIN PROGRAM FROM HERE */

// Almost done!

void main () {
	// Main contains pretty standard stuff..

	// turn off the LED
	anypio_write(LED, false);

	// configure KEY1 with a PULL_UP resistor so we can use it as a
	// button.
	anypio_digital_input_set(KEY1_REV2, PULL_UP);

	// The following code initializes the USB subsystem by inserting our
	// device descriptors into the USB_Device_Struct and telling the
	// hardware to reset and connect. 

	USB_Init(&midi_deviceDefinition, &midi_device);
	USB_SoftConnect (&midi_device);
	
	// Finally we start the Systick Interrupt which runs every 10ms.
	// The routine called by this interrupt is defined next ...
	SYSCON_StartSystick_10ms();

	// That's it for main, the anykey will just hang around waiting for
	// keypresses in the systick routine and wait for midi data to arrive
	// from the host. This data is handled by the callbacks declared above
	// (myMidiNoteOnHandler) and implemented below ...
}

bool lastButton = false;

void systick () {
	// read the button value
	bool button = !anypio_read(KEY1_REV2);
	// check if the button state has changed since the last time we
	// looked.
	if (button != lastButton) {
		if (button) {
			// if the button was pressed, we send a note-on command to:
			// cable  :  3 (defined in the device descriptor)
			// channel:  1 
			// note   : 60 (middle C)
			// velocity:127 (play loud!)
			USBMIDI_SendNoteOn(&midi_device, &midi_behaviour, 3, 1, 60, 127);
		} else {
			// when the button is released, we turn off our middle C:
			USBMIDI_SendNoteOff(&midi_device, &midi_behaviour, 3, 1, 60);
		}
	}
	lastButton = button;
}

// whenever we receive a note on message, we turn on our LED ...
void myMidiNoteOnHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note, uint8_t velocity) {
	anypio_write(LED, velocity > 0);
}

// ... and turn it off again on any note off message.
void myMidiNoteOffHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note) {
	anypio_write(LED, false);
}

// ignore fancy stuff ...
void myMidiControlChangeHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t control, uint8_t value) {
}

// We'll be doing some magic with sysex messages... The following is a
// forward declaration to some code we've factored out of the sysex
// handler for clarify.
void parseSysEx(uint8_t* data, uint8_t length);

// Sysex (System Exclusive) messages are part of the MIDI standard and
// used to transport arbitrary binary data. The Handler below will
// receive sysex data, prepend a "Hello" to it and send the data back:
// a MIDI "Hello World" !

void myMidiSysExHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t* data, uint8_t length, bool moreToCome) {
	static uint8_t sysExData[MAX_SYSEX_SIZE];
	static uint8_t sysExLength = 0;

	uint8_t remaining = MAX_SYSEX_SIZE - sysExLength;
	if (remaining > length)	{
		uint8_t i;
		for (i=0; i<length; i++) {
			sysExData[sysExLength+i] = data[i];
		}
		sysExLength += length;
	}
	if (!moreToCome) {
		parseSysEx(sysExData, sysExLength);
		sysExLength = 0;
	}
}

void parseSysEx(uint8_t* data, uint8_t length) {
	if (length < 4) return;
	if (length > 200) return;
	if (data[0] != 0xf0) return;
	if (data[1] != 0x7f) return;
	if (data[2] != 0x7e) return;
	if (data[length-1] != 0xf7) return;
	uint8_t i;
	for (i=length-4; i>0; i--) {
		data[8+i] = data[2+i];
	}
	data[3] = 'H';
	data[4] = 'e';
	data[5] = 'l';
	data[6] = 'l';
	data[7] = 'o';
	data[8] = ' ';
	length += 6;
	data[length-1] = 0xf7;
	USBMIDI_SendSysEx(&midi_device, &midi_behaviour, 3, data, length);
}
