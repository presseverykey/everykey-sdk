#include "anykey/anykey.h"
#include "anykey_usb/midi.h"
#include "anypio.h"


#define DATA_OUT_ENDPOINT_LOGICAL 0x01
#define DATA_OUT_ENDPOINT_PHYSICAL 2
#define DATA_IN_ENDPOINT_LOGICAL 0x81
#define DATA_IN_ENDPOINT_PHYSICAL 3
#define MIDI_FIFO_SIZE 256
#define MAX_SYSEX_SIZE 255

USB_Device_Struct midi_device;

uint8_t inBuffer[USB_MAX_BULK_DATA_SIZE];				//data from device to host
uint8_t outBuffer[USB_MAX_BULK_DATA_SIZE];				//data from host to device
uint8_t midiFifo[MIDI_FIFO_SIZE];
uint16_t midiFifoRdIdx = 0;
uint16_t midiFifoWrIdx = 0;

void myMidiNoteOnHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note, uint8_t velocity);
void myMidiNoteOffHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note);
void myMidiControlChangeHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t control, uint8_t value);
void myMidiSysExHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t* data, uint8_t length, bool moreToCome);

void parseSysEx(uint8_t* data, uint8_t length);

const uint8_t midi_deviceDescriptor[] = {

	0x12,                           //bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,                //bDescriptorType: usb device descriptor
	I16_TO_LE_BA(0x0200),           //bcdUSB: 0200 (Little Endian) - USB 2.0 compliant
	USB_CLASS_DEVICE,               //bDeviceClass: Device class (0 = interfaces specify class)
	0x00,                           //bDeviceSubClass: Device subclass (must be 0 if bDeviceClass is 0)
	0x00,                           //bDeviceProtocol: 0 for no specific device-level protocols
	USB_MAX_COMMAND_PACKET_SIZE,    //bMaxPacketSize0: Max packet size for control endpoint
	I16_TO_LE_BA(0x1234),           //idVendor: 16 bit vendor id
	I16_TO_LE_BA(0x5678),           //idProduct: 16 bit product id
	I16_TO_LE_BA(0x0100),           //bcdDevice: Device release version
	0x01,                           //iManufacturer: Manufacturer string index
	0x02,                           //iProduct: Product string index
	0x03,                           //iSerialNumber: Serial number string index
	0x01                            //bNumConfigurations: Number of configurations
};

const uint8_t midi_configDescriptor[] = {
	//Config descriptor
	9,                              //bLength
	USB_DESC_CONFIGURATION,         //bDescriptorType
	I16_TO_LE_BA(101),              //wTotalLength (configDesc complete)
	2,                              //bNumInterfaces
	1,                              //bConfigurationValue
	0,                              //iConfiguration
	0x80,                           //bmAttributes : bus powered
	50,                             //bMaxPower : 100mA

	//Standard Audio Class Interface Descriptor
	0x09,                   // bLength - ￼Size of this descriptor, in bytes.
	0x04,                   // bDescriptorType - INTERFACE descriptor.
	0x00,                   // bInterfaceNumber - Index of this interface.
	0x00,                   // bAlternateSetting - Index of this setting.
	0x00,                   // bNumEndpoints - 0 endpoints.
	0x01,                   // bInterfaceClass - AUDIO.
	0x01,                   // bInterfaceSubclass - AUDIO_CONTROL.
	0x00,                   // bInterfaceProtocol - ￼Unused.
	0x00,                   // iInterface - ￼Unused.

	//Class-specific Audio Class Interface Descriptor
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - 
	0x01,                   // bDescriptorSubtype - HEADER subtype.
	I16_TO_LE_BA(0x0100),   //bcdADC - Revision of class specification - 1.0
	I16_TO_LE_BA(0x0009),   // wTotalLength - Total size of class specific descriptors.
	0x01,                   // bInCollection - Number of streaming interfaces.
	0x01,                   // baInterfaceNr(1) - MIDIStreaming interface 1 belongs to this AudioControl interface.

	//Standard MIDI Streaming Interface Descriptor
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x04,                   // bDescriptorType - INTERFACE descriptor.
	0x01,                   // bInterfaceNumber - Index of this interface.
	0x00,                   // bAlternateSetting - Index of this alternate setting.
	0x02,                   // bNumEndpoints - 2 endpoints.
	0x01,                   // bInterfaceClass - AUDIO.
	0x03,                   // bInterfaceSubclass - MIDISTREAMING.
	0x00,                   // bInterfaceProtocol - Unused.
	0x00,                   // iInterface - Unused.

	// Class-specific MIDI Streaming Interface Descriptor
	0x07,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x01,                   // bDescriptorSubtype - MS_HEADER subtype.
	I16_TO_LE_BA(0x0100),   // BcdADC - ￼Revision of this class specification.
	I16_TO_LE_BA(37),   	// wTotalLength - ￼Total size of class-specific descriptors. Example says 0x41, but doc says this and all elements and jacks

	//MIDI IN Jack Descriptor (Embedded)
	0x06,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x02,                   // bDescriptorSubtype - MIDI_IN_JACK subtype.
	0x01,                   // bJackType - EMBEDDED.
	0x01,                   // bJackID - ID of this Jack.
	0x00,                   // iJack - Unused.

	//MIDI Adapter MIDI IN Jack Descriptor (External)
	0x06,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x02,                   // bDescriptorSubtype - MIDI_IN_JACK subtype.
	0x02,                   // bJackType - EXTERNAL .
	0x02,                   // bJackID - ID of this Jack.
	0x00,                   // iJack - Unused.

	//MIDI OUT Jack Descriptor (Embedded)
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x03,                   // bDescriptorSubtype - MIDI_OUT_JACK subtype.
	0x01,                   // bJackType - EMBEDDED.
	0x03,                   // bJackID - ID of this Jack.
	0x01,                   // bNrInputPins - Number of Input Pins of this Jack.
	0x02,                   // BaSourceID(1) - ID of the Entity to which this Pin is connected.
	0x01,                   // BaSourcePin(1) - Output Pin number of the Entity to which this Input Pin is connected.
	0x00,                   // iJack - Unused.

	//MIDI Adapter MIDI OUT Jack Descriptor (External)
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x24,                   // bDescriptorType - CS_INTERFACE descriptor.
	0x03,                   // bDescriptorSubtype - MIDI_OUT_JACK subtype.
	0x02,                   // bJackType - EXTERNAL.
	0x04,                   // bJackID - ID of this Jack.
	0x01,                   // bNrInputPins - Number of Input Pins of this Jack.
	0x01,                   // BaSourceID(1) - ID of the Entity to which this Pin is connected.
	0x01,                   // BaSourcePin(1) - Output Pin number of the Entity to which this Input Pin is connected.
	0x00,                   // iJack - Unused.

	//Standard Bulk OUT Endpoint Descriptor
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x05,                   // bDescriptorType - ENDPOINT descriptor.
	0x01,                   // bEndpointAddress - OUT Endpoint 1.
	0x02,                   // bmAttributes - Bulk, not shared.
	I16_TO_LE_BA(0x0040),   // wMaxPacketSize - 64 bytes per packet.
	0x00,                   // bInterval - Ignored for Bulk. Set to zero.
	0x00,                   // bRefresh - Unused.
	0x00,                   // bSynchAddress - Unused.

	//Class-specific MS Bulk OUT Endpoint Descriptor
	0x05,                   // bLength - Size of this descriptor, in bytes.
	0x25,                   // bDescriptorType - CS_ENDPOINT descriptor
	0x01,                   // bDescriptorSubtype - MS_GENERAL subtype.
	0x01,                   // bNumEmbMIDIJack - Number of embedded MIDI IN Jacks.
	0x01,                   // BaAssocJackID(1) - ID of the Embedded MIDI IN Jack.

	//Standard Bulk IN Endpoint Descriptor
	0x09,                   // bLength - Size of this descriptor, in bytes.
	0x05,                   // bDescriptorType - ENDPOINT descriptor.
	0x81,                   // bEndpointAddress - IN Endpoint 1.
	0x02,                   // bmAttributes - Bulk, not shared.
	I16_TO_LE_BA(0x0040),   // wMaxPacketSize - 64 bytes per packet.
	0x00,                   // bInterval - Ignored for Bulk. Set to zero.
	0x00,                   // bRefresh - Unused.
	0x00,                   // bSynchAddress - Unused.

	//Class-specific MS Bulk IN Endpoint Descriptor
	0x05,                   // bLength - Size of this descriptor, in bytes.
	0x25,                   // bDescriptorType - CS_ENDPOINT descriptor
	0x01,                   // bDescriptorSubtype - MS_GENERAL subtype.
	0x01,                   // bNumEmbMIDIJack - Number of embedded MIDI OUT Jacks.
	0x03                    // BaAssocJackID(1) - ID of the Embedded MIDI OUT Jack.};
};

const uint8_t midi_languages[] = {
	0x04,								//bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	I16_TO_LE_BA(0x0409)				//wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};
	
const uint8_t midi_manufacturerName[] = {
	0x22,								//bLength: length of this descriptor in bytes (34)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'P',0,'r',0,'e',0,'s',0,'s',0,' ',0,'A',0,'n',0,'y',0,' ',0,'K',0,'e',0,'y',0,' ',0,'U',0,'G',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t midi_deviceName[] = {
	0x12,								//bLength: length of this descriptor in bytes (18)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,'0',0,'x',0			//bString[]: String (UTF16LE, not terminated)
};

const uint8_t midi_serialName[] = {
	0x0a,								//bLength: length of this descriptor in bytes (10)
	USB_DESC_STRING,					//bDescriptorType: string descriptor
	'V',0,'1',0,'.',0,'0',0				//bString[]: String (UTF16LE, not terminated)
};

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


/** MAIN PROGRAM FROM HERE */

void main () {
	anypio_write(LED, false);
	anypio_digital_input_set(KEY1_REV2, PULL_UP);

	USB_Init(&midi_deviceDefinition, &midi_device);

	USB_SoftConnect (&midi_device);

	SYSCON_StartSystick_10ms();
}

bool lastButton = false;

void systick () {
	bool button = !anypio_read(KEY1_REV2);
	if (button != lastButton) {
		if (button) USBMIDI_SendNoteOn(&midi_device, &midi_behaviour, 3, 1, 60, 127);
		else USBMIDI_SendNoteOff(&midi_device, &midi_behaviour, 3, 1, 60);
	}
	lastButton = button;
}

void myMidiNoteOnHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note, uint8_t velocity) {
	anypio_write(LED, velocity > 0);
}

void myMidiNoteOffHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t note) {
	anypio_write(LED, false);
}

void myMidiControlChangeHandler(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableIdx, uint8_t channel, uint8_t control, uint8_t value) {
}


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
	data[4] = 'a';
	data[5] = 'l';
	data[6] = 'l';
	data[7] = 'o';
	data[8] = ' ';
	length += 6;
	data[length-1] = 0xf7;
	USBMIDI_SendSysEx(&midi_device, &midi_behaviour, 3, data, length);
}
