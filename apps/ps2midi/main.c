#include "anykey/anykey.h"
#include "usbmidiboilerplate.h"
#include "ps2app.h"


#define LED_PORT 0
#define LED_PIN 7

//MIDI STUFF FROM HERE

#define MIDI_FIFO_SIZE 256
#define MAX_SYSEX_SIZE 128

#define TOGGLE_LED any_gpio_write(0,7,!any_gpio_read(0,7));

uint8_t inBuffer[USB_MAX_BULK_DATA_SIZE];				//data from device to host
uint8_t outBuffer[USB_MAX_BULK_DATA_SIZE];			//data from host to device
uint8_t midiFifo[MIDI_FIFO_SIZE];
uint16_t midiFifoRdIdx = 0;
uint16_t midiFifoWrIdx = 0;

uint8_t connectionState = 0;	//bitmask: 0x01=mouse, 0x02=keyboard


void myMidiNoteOnHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note, uint8_t velocity);
void myMidiNoteOffHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note);
void myMidiControlChangeHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t control, uint8_t value);
void myMidiSysExHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t* data, uint8_t length, bool moreToCome);

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

const USB_Device_Definition midi_deviceDefinition = {
        midi_deviceDescriptor,
        1,
        { midi_configDescriptor },
        4,
        { midi_languages, midi_manufacturerName, midi_deviceName, midi_serialName },
        1,
        { (USB_Behaviour_Struct*)(&midi_behaviour) }
};

USB_Device_Struct midi_device;

//PS/2 STUFF FROM HERE
void PS2ConnectionChange(PS2_APP_CONNECTION newState);
void PS2MouseInput(int16_t dx, int16_t dy, int16_t dz, bool left, bool right, bool middle);
void PS2KeyboardInput(uint8_t keycode, bool down, uint8_t leds);
void PS2Idle();
void PS2Debug(uint8_t* string);

// MAIN PROGRAM FROM HERE

void main () {
	any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	any_gpio_write(LED_PORT, LED_PIN, false);

	USB_Init(&midi_deviceDefinition, &midi_device);
	USB_SoftConnect (&midi_device);
	
	ANY_GPIO_SET_FUNCTION(0,4,PIO,IOCON_IO_ADMODE_DIGITAL);
	ANY_GPIO_SET_FUNCTION(0,5,PIO,IOCON_IO_ADMODE_DIGITAL);
	any_gpio_set_dir(0,4,INPUT);
	any_gpio_set_dir(0,5,INPUT);

	ps2app_init(PS2ConnectionChange, PS2MouseInput, PS2KeyboardInput, PS2Idle);
}

//MIDI SYSEX SENDING STUFF

typedef enum {
	CONNECTION_MSG = 0x01,	//1 byte: bit 0: mouse connected, bit 1: keyboard connected
	MOUSE_EVENT_MSG = 0x02, //7 bytes: flags, dx hi/lo, dy hi/lo, dz hi/lo
	KEY_EVENT_MSG = 0x03,	//2 bytes: code + down
	DEBUG_STRING_MSG = 0x7f //null-terminated string
} PS2MIDI_MESSAGE_TYPE;

void sendDebugString(uint8_t* string) {
	uint8_t len = 0;
	while (string[len]) len++;
	uint8_t data[len+8];
	data[0] = 0xf0;
	data[1] = 0x7d;
	data[2] = 'A';
	data[3] = 'n';
	data[4] = 'y';
	data[5] = 1;
	data[6] = DEBUG_STRING_MSG;
	int i = 0;
	for (i=0; i<len; i++) data[7+i] = string[i];
	data[len+7] = 0xf7;
	USBMIDI_SendSysEx(&midi_device, &midi_behaviour, 3, data, sizeof(data));
}

void sendConnectionState() {
	uint8_t data[] = {
		0xf0, 0x7d,
		'A','n','y',1,		//Magic + version number
		CONNECTION_MSG, connectionState,
		0xf7
	};
	USBMIDI_SendSysEx(&midi_device, &midi_behaviour, 3, data, sizeof(data));
}

void sendMouseEvent(int16_t dx, int16_t dy, int16_t dz, bool left, bool right, bool middle) {
	uint8_t flags = (left   ? 0x01 : 0) + 
					(right  ? 0x02 : 0) + 
					(middle ? 0x04 : 0);
	uint8_t data[] = {
		0xf0, 0x7d,
		'A','n','y',1,		//Magic + version number
		MOUSE_EVENT_MSG,
		flags,
		((uint8_t*)(&dx))[0],((uint8_t*)(&dx))[1],
		((uint8_t*)(&dy))[0],((uint8_t*)(&dy))[1],
		((uint8_t*)(&dz))[0],((uint8_t*)(&dz))[1],
		0xf7
	};
	USBMIDI_SendSysEx(&midi_device, &midi_behaviour, 3, data, sizeof(data));
}

void sendKeyboardEvent(uint8_t keycode, bool down, uint8_t leds) {
	uint8_t data[] = {
		0xf0, 0x7d,
		'A','n','y',1,		//Magic + version number
		KEY_EVENT_MSG,
		keycode, down ? 1 : 0, leds,
		0xf7
	};
	USBMIDI_SendSysEx(&midi_device, &midi_behaviour, 3, data, sizeof(data));
}

//PS/2 EVENT HANDLERS

void PS2ConnectionChange(PS2_APP_CONNECTION newState) {
	connectionState = 	((newState == PS2_APP_CONN_RUN_MOUSE) ? 1 : 0) |
						((newState == PS2_APP_CONN_RUN_KEYBOARD) ? 2 : 0);
	sendConnectionState();
}

void PS2MouseInput(int16_t dx, int16_t dy, int16_t dz, bool left, bool right, bool middle) {
	sendMouseEvent(dx, dy, dz, left, right, middle);
}

void PS2KeyboardInput(uint8_t keycode, bool down, uint8_t leds) {
	sendKeyboardEvent(keycode, down, leds);
}

void PS2Idle() {
//	any_gpio_write(LED_PORT, LED_PIN, connectionState != 0);
	sendConnectionState();
}

// MIDI EVENT HANDLERS (we don't do that...)

void myMidiNoteOnHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note, uint8_t velocity) {
	//IGNORE
}

void myMidiNoteOffHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note) {
	//IGNORE
}

void myMidiControlChangeHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t control, uint8_t value) {
	//IGNORE
}

void myMidiSysExHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t* data, uint8_t length, bool moreToCome) {
	//IGNORE
}

