#include "pressanykey/pressanykey.h"
#include "pressanykey_usb/usb.h"
#include "pressanykey_usb/usbaudiospec.h"

#define AUDIO_CONTROL_INTERFACE 0

#define SPEAKER_STREAM_INTERFACE 1
#define MIC_STREAM_INTERFACE 2

#define SPEAKER_INTERM_NODE 1
#define SPEAKER_FEATURE_NODE 2
#define SPEAKER_OUTTERM_NODE 3
#define MIC_INTERM_NODE 4
#define MIC_FEATURE_NODE 5
#define MIC_OUTTERM_NODE 6
#define SAMPLE_RATE 48000
#define BYTES_PER_FRAME (((SAMPLE_RATE/1000)+1)*2) /* samples per ms * sample size, 1 sample extra */

#define KEY_PORT 1
#define KEY_PIN 4
#define LED_PORT 0
#define LED_PIN 7

const uint8_t deviceDescriptor[] = {
	0x12,							//bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,				//bDescriptorType: usb device descriptor
	I16_TO_LE_BA(0x0200),			//bcdUSB: USB 2.0
	0x00,							//bDeviceClass: Device class (0 = interfaces specify class)
	0x00,							//bDeviceSubClass: Device subclass (must be 0 if bDeviceClass is 0)
	0x00,							//bDeviceProtocol: 0 for no specific device-level protocols
	USB_MAX_COMMAND_PACKET_SIZE,	//bMaxPacketSize0: Max packet size for control endpoint
	I16_TO_LE_BA(0x1234),           //idVendor: 16 bit vendor id
	I16_TO_LE_BA(0x4567),           //idProduct: 16 bit product id
	I16_TO_LE_BA(0x0100),			//bcdDevice: Device release version
	0x01,                           //iManufacturer: Manufacturer string index
	0x02,                           //iProduct: Product string index
	0x03,                           //iSerialNumber: Serial number string index
	0x01                            //bNumConfigurations: Number of configurations
};

const uint8_t languages[] = {
	0x04,							//bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	0x09,0x04						//wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};

const uint8_t manufacturerName[] = {
	0x14,							//bLength: length of this descriptor in bytes (20)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,' ',0,'U',0,'G',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t deviceName[] = {
	0x12,							//bLength: length of this descriptor in bytes (30)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,'0',0,'x',0,' ',0,'A',0,'u',0,'d',0,'i',0,'o',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t serialName[] = {
	0x0a,							//bLength: length of this descriptor in bytes (10)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'V',0,'1',0,'.',0,'0',0			//bString[]: String (UTF16LE, not terminated)
};


/** Now the most annoying part of USB Audio: The configuration decriptor. The spec requires an extensive
 * description of the device. Here's the short version: The configuration descriptor is
 * generally structured as usual, but the spec requires additional class-specific descriptors inbetween
 * the standard ones. USB Audio devices consist of two parts: One Audio Control interface and one or more
 * Audio Stream interfaces. The control interface mainly describes a graph of audio processing nodes.
 * Terminal nodes can either lead to the host (via USB - hey, that's what the whole thing is about) or to non-USB
 * terminals (microphones, loudspeakers etc.). Each unit has a unique ID that can be used to communicate to
 * it via the control interface (setting volume etc.). Each USB terminal node has an Audio Stream interface
 * that is responsible for transmitting the actual audio data. These interfaces usually contain an
 * isochronous endpoint for audio streaming. The USB spec requires interfaces with isochronous endpoints
 * to provide a default zero-bandwidth alternate setting. Hence, most Audio Streaming interfaces
 * come with (at least) two alternate settings: 0 without isoch endpoint, 1 with isoch endpoint. The additional
 * descriptors in stream interfaces mainly describe the corresponding node in the processing graph and the
 * audio stream format (usually PCM). */

const uint8_t configDescriptor[] = {
	//Config descriptor
	9,													//bLength
	USB_DESC_CONFIGURATION,								//bDescriptorType
	I16_TO_LE_BA(196),									//wTotalLength (configDesc complete)
	3,													//bNumInterfaces
	1,													//bConfigurationValue
	0,													//iConfiguration
	0x80,												//bmAttributes
	50,													//bMaxPower
	
	//Interface 0: Audio control
	9,													//bLength
	USB_DESC_INTERFACE,									//bDescriptorType
	AUDIO_CONTROL_INTERFACE,							//bInterfaceNumber
	0,													//bAlternateSetting
	0,													//bNumEndpoints
	USB_CLASS_AUDIO,									//bInterfaceClass
	USB_AUDIO_INTERFACE_AUDIOCONTROL,					//bInterfaceSubclass
	0,													//bInterfaceProtocol
	0,													//iInterface
	
	//Audio control header (mandatory for audio control interfaces)
	10,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_CONTROL_INTERFACE_DESCRIPTOR_HEADER,		//bDescriptorSubtype
	I16_TO_LE_BA(0x0100),								//bcdADC
	I16_TO_LE_BA(74),									//wTotalLength (this+units+terminals)
	2,													//nInConnection
	SPEAKER_STREAM_INTERFACE,							//baInterfaceNr1
	MIC_STREAM_INTERFACE,								//baInterfaceNr2
	
	// The next 6 descriptors specify the audio setup as nodes. Here's the short version:
	// 1 USB out stream (IN TERMINAL) -> 2 out feature (FEATURE) -> 3 speaker (OUT TERMINAL)
	// 6 USB in stream (OUT TERMINAL) <- 5 in feature  (FEATURE) <- 4 microphone (IN TERMINAL)
	
	// 1 USB out stream node (USB Audio Spec 1.0 section 4.3.2.2)
	12,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_CONTROL_INTERFACE_DESCRIPTOR_INPUT_TERMINAL,	//bDescriptorSubtype
	SPEAKER_INTERM_NODE,								//bTerminalId
	I16_TO_LE_BA(USB_AUDIO_TERMINAL_USB_STREAMING),		//wTerminalType
	0,													//bAssocTerminal
	1,													//bNrChannels
	I16_TO_LE_BA(USB_AUDIO_CHANNELCONFIG_LEFT_FRONT),	//wChannelConfig (should use center front, just for fun here)
	0,													//iChannelNames (none)
	0,													//iTerminal (none)
	
	// 2 out feature node (USB Audio Spec 1.0 section 4.3.2.5)
	11,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_CONTROL_INTERFACE_DESCRIPTOR_FEATURE_UNIT,	//bDescriptorSubtype
	SPEAKER_FEATURE_NODE,								//bUnitID
	SPEAKER_INTERM_NODE,								//bSourceID
	2,													//bControlSize
	I16_TO_LE_BA(USB_AUDIO_FEATURE_VOLUME_FLAG),		//bmaControls 0 : Master -> Volume
	I16_TO_LE_BA(0),									//bmaControls 1 : chan 1 -> none
	0,													//iFeature : no string
	
	// 3 speaker node (USB Audio Spec 1.0 section 4.3.2.1)
	9,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_CONTROL_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL,	//bDescriptorSubtype
	SPEAKER_OUTTERM_NODE,								//bTerminalId
	I16_TO_LE_BA(USB_AUDIO_TERMINAL_OUT_SPEAKER),		//wTerminalType (speaker)
	0,													//bAssocTerminal
	SPEAKER_FEATURE_NODE,								//bSourceID
	0,													//iTerminal (none)
	
	// 4 mic (USB Audio Spec 1.0 section 4.3.2.2)
	12,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_CONTROL_INTERFACE_DESCRIPTOR_INPUT_TERMINAL,	//bDescriptorSubtype
	MIC_INTERM_NODE,									//bTerminalId
	I16_TO_LE_BA(USB_AUDIO_TERMINAL_IN_MICROPHONE),		//wTerminalType
	0,													//bAssocTerminal
	1,													//bNrChannels
	I16_TO_LE_BA(USB_AUDIO_CHANNELCONFIG_CENTER_FRONT),	//wChannelConfig
	0,													//iChannelNames (none)
	0,													//iTerminal (none)
	
	// 5 in feature node (USB Audio Spec 1.0 section 4.3.2.5)
	11,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_CONTROL_INTERFACE_DESCRIPTOR_FEATURE_UNIT,	//bDescriptorSubtype
	MIC_FEATURE_NODE,									//bUnitID
	MIC_INTERM_NODE,									//bSourceID
	2,													//bControlSize
	I16_TO_LE_BA(USB_AUDIO_FEATURE_VOLUME_FLAG),		//bmaControls 0 : Master -> Volume
	I16_TO_LE_BA(0),									//bmaControls 1 : chan 1 -> none
	0,													//iFeature : no string
	
	// 6 USB in stream node (USB Audio Spec 1.0 section 4.3.2.1)
	9,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_CONTROL_INTERFACE_DESCRIPTOR_OUTPUT_TERMINAL,	//bDescriptorSubtype
	MIC_OUTTERM_NODE,									//bTerminalId
	I16_TO_LE_BA(USB_AUDIO_TERMINAL_USB_STREAMING),		//wTerminalType
	0,													//bAssocTerminal
	MIC_FEATURE_NODE,									//bSourceID
	0,													//iTerminal (none)
	
	//Interface 1 alt 0: Audio stream out (speaker, no bandwidth)
	9,													//bLength
	USB_DESC_INTERFACE,									//bDescriptorType
	SPEAKER_STREAM_INTERFACE,							//bInterfaceNumber
	0,													//bAlternateSetting
	0,													//bNumEndpoints
	USB_CLASS_AUDIO,									//bInterfaceClass
	USB_AUDIO_INTERFACE_AUDIOSTREAMING,					//bInterfaceSubclass
	0,													//bInterfaceProtocol
	0,													//iInterface
	
	//Interface 1 alt 1: Audio stream out (speaker, with bandwidth)
	9,													//bLength
	USB_DESC_INTERFACE,									//bDescriptorType
	SPEAKER_STREAM_INTERFACE,							//bInterfaceNumber
	1,													//bAlternateSetting
	1,													//bNumEndpoints
	USB_CLASS_AUDIO,									//bInterfaceClass
	USB_AUDIO_INTERFACE_AUDIOSTREAMING,					//bInterfaceSubclass
	0,													//bInterfaceProtocol
	0,													//iInterface
	
	//Audio Class Audio Stream Interface descriptor (USB Audio Spec 1.0 section 4.5.2)
	7,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_STREAM_INTERFACE_DESCRIPTOR_GENERAL,		//bDescriptorSubtype
	SPEAKER_INTERM_NODE,								//bTerminalLink
	0,													//bDelay
	I16_TO_LE_BA(USB_AUDIO_FORMAT_I_PCM),				//wFormatFlag
	
	//Audio Class Audio Stream Format Type (USB Audio Format spec type 1, section 2.2.5
	11,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_STREAM_INTERFACE_DESCRIPTOR_FORMAT_TYPE,	//bDescriptorSubtype
	USB_AUDIO_FORMATTYPE_I,								//bFormatType: Type 1 (PCM-like)
	1,													//bNrChannels: 1/mono
	2,													//bSubframeSize: 2 bytes per sample
	16,													//bBitResolution: We state 16 although we use less
	1,													//bSamFreqType: Only one fixed freq
	I24_TO_LE_BA(SAMPLE_RATE),							//tSamFreq[0]
	
	//Standard AS Isochronous out endpoint descriptor (see audio spec 4.6.1.1)
	9,													//bLength
	USB_DESC_ENDPOINT,									//bDescriptorType
	0x04,												//bEndpointAddress:4 out (our isoch out endpoint)
	USB_EPTYPE_ISOCHRONOUS | USB_EPSYNC_ADAPTIVE,		//bmAttributes
	I16_TO_LE_BA(BYTES_PER_FRAME),						//wMaxPacketSize
	1,													//bInterval: isoch = every frame
	0,													//bRefresh (always 0)
	0,													//unused by spec (just to make length 9 fit)
	
	//Audio Class Isochronous Audio Data Endpoint Descriptor (USB Audio Spec 1.0, section 4.6.1.2)
	7,													//bLength
	USB_DESC_AUDIO_ENDPOINT,							//bDescriptorType
	USB_AUDIO_ENDPOINT_DESCRIPTOR_GENERAL,				//bDescriptorSubtype
	USB_AUDIO_EP_ATTRIBUTE_FREQ,						//bmAttributes (none)
	USB_AUDIO_LOCKDELAY_SAMPLES,						//bLockDelayUnits
	I16_TO_LE_BA(1),									//bLockDelay
	
	
	//Interface 2 alt 0: Audio stream in (mic, no bandwidth)
	9,													//bLength
	USB_DESC_INTERFACE,									//bDescriptorType
	MIC_STREAM_INTERFACE,								//bInterfaceNumber
	0,													//bAlternateSetting
	0,													//bNumEndpoints
	USB_CLASS_AUDIO,									//bInterfaceClass
	USB_AUDIO_INTERFACE_AUDIOSTREAMING,					//bInterfaceSubclass
	0,													//bInterfaceProtocol
	0,													//iInterface
	
	
	//Interface 2 alt 1: Audio stream in (mic, with bandwidth)
	9,													//bLength
	USB_DESC_INTERFACE,									//bDescriptorType
	MIC_STREAM_INTERFACE,								//bInterfaceNumber
	1,													//bAlternateSetting
	1,													//bNumEndpoints
	USB_CLASS_AUDIO,									//bInterfaceClass
	USB_AUDIO_INTERFACE_AUDIOSTREAMING,					//bInterfaceSubclass
	0,													//bInterfaceProtocol
	0,													//iInterface
	
	//Audio Class Audio Stream Interface descriptor (USB Audio Spec 1.0 section 4.5.2)
	7,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_STREAM_INTERFACE_DESCRIPTOR_GENERAL,		//bDescriptorSubtype
	MIC_OUTTERM_NODE,									//bTerminalLink
	0,													//bDelay
	I16_TO_LE_BA(USB_AUDIO_FORMAT_I_PCM),				//wFormatFlag
	
	//Audio Class Audio Stream Format Type (USB Audio Format spec type 1, section 2.2.5
	11,													//bLength
	USB_DESC_AUDIO_INTERFACE,							//bDescriptorType
	USB_AUDIO_STREAM_INTERFACE_DESCRIPTOR_FORMAT_TYPE,	//bDescriptorSubtype
	USB_AUDIO_FORMATTYPE_I,								//bFormatType: Type 1 (PCM-like)
	1,													//bNrChannels: 1/mono
	2,													//bSubframeSize: 2 bytes per sample
	16,													//bBitResolution: We state 16 although we use less
	1,													//bSamFreqType: Only one fixed freq
	I24_TO_LE_BA(SAMPLE_RATE),							//tSamFreq[0]
	
	//Standard AS Isochronous out endpoint descriptor (see audio spec 4.6.1.1)
	9,													//bLength
	USB_DESC_ENDPOINT,									//bDescriptorType
	0x84,												//bEndpointAddress:4 in (our isoch in endpoint)
	USB_EPTYPE_ISOCHRONOUS | USB_EPSYNC_ADAPTIVE,		//bmAttributes
	I16_TO_LE_BA(BYTES_PER_FRAME),						//wMaxPacketSize (72 should be sufficient)
	1,													//bInterval: isoch = every frame
	0,													//bRefresh (always 0)
	0,													//unused by spec (just to make length 9 fit)
	
	//Audio Class Isochronous Audio Data Endpoint Descriptor (USB Audio Spec 1.0, section 4.6.1.2)
	7,													//bLength
	USB_DESC_AUDIO_ENDPOINT,							//bDescriptorType
	USB_AUDIO_ENDPOINT_DESCRIPTOR_GENERAL,				//bDescriptorSubtype
	USB_AUDIO_EP_ATTRIBUTE_FREQ,						//bmAttributes (none)
	USB_AUDIO_LOCKDELAY_SAMPLES,						//bLockDelayUnits
	I16_TO_LE_BA(1)										//bLockDelay
};


/** called when the host wants to write a control value. 
 @param request indicates which type is to be set. Usually only SET_CUR.
 @param nodeId node containing the control to set
 @param channelIdx index of channel to set
 @param selector selector of control to set
 @param channelId affected channel or 0 for master
 @param paramBlock pointer to buffer containing the value to be set
 @param paramBlockLength length of param block
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_SetControlValue)(USB_AUDIO_REQUEST request,
										 uint8_t nodeId,
										 uint8_t channelId,
										 USB_AUDIO_CONTROL_SELECTOR selector,
										 uint8_t* paramBlock,
										 uint8_t paramBlockLength);

/** called when the host wants to read a control value. 
 @param request indicates which type is to be read. GET_CUR, GET_MIN, GET_MAX or GET_RES.
 @param nodeId node containing the control to be read
 @param channelIdx index of channel to read
 @param selector selector of control to be read
 @param channelId affected channel or 0 for master
 @param paramBlock pointer to buffer to be filled with return value
 @param paramBlockLength length to fill
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_GetControlValue)(USB_AUDIO_REQUEST request,
										 uint8_t nodeId,
										 uint8_t channelId,
										 USB_AUDIO_CONTROL_SELECTOR selector,
										 uint8_t* paramBlock,
										 uint8_t paramBlockLength);

/** called when the host wants to write an endpoint value. 
 @param request indicates which type is to be set. Usually only SET_CUR.
 @param endpoint the endpoint number (USB logical)
 @param selector selector of control to be set
 @param paramBlock pointer to buffer containing the value to be set
 @param paramBlockLength length of param block
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_SetEndpointValue)(USB_AUDIO_REQUEST request,
										  uint8_t endpoint,
										  USB_AUDIO_CONTROL_SELECTOR selector,
										  uint8_t* paramBlock,
										  uint8_t paramBlockLength);

/** called when the host wants to read an endpoint value. 
 @param request indicates which type is to be read. GET_CUR, GET_MIN, GET_MAX or GET_RES.
 @param endpoint the endpoint number (USB logical)
 @param selector selector of control to be read
 @param paramBlock pointer to buffer to be filled with return value
 @param paramBlockLength length to fill
 @return true if the request was handled, false otherwise */
typedef bool (*USBAudio_GetEndpointValue)(USB_AUDIO_REQUEST request,
										  uint8_t endpoint,
										  USB_AUDIO_CONTROL_SELECTOR selector,
										  uint8_t* paramBlock,
										  uint8_t paramBlockLength);


int16_t speakerVolume;
int16_t micVolume;

bool AudioGetControlValue(USB_AUDIO_REQUEST request,
						  uint8_t nodeId,
						  uint8_t channelId,
						  USB_AUDIO_CONTROL_SELECTOR selector,
						  uint8_t* paramBlock,
						  uint8_t paramBlockLength) {
	switch (nodeId) {
		case SPEAKER_FEATURE_NODE:
			if (channelId == 0) {
				if (selector == USB_AUDIO_CSEL_FEATURE_VOLUME) {
					if (paramBlockLength == 2) {
						switch (request) {
							case USB_AUDIO_REQ_GET_CUR:
								paramBlock[1] = (speakerVolume>>8) & 0xff;
								paramBlock[0] = speakerVolume & 0xff;
								return true;
								break;
							case USB_AUDIO_REQ_GET_MIN:
								paramBlock[1] = 0x80; paramBlock[0] = 0x00;
								return true;
								break;
							case USB_AUDIO_REQ_GET_MAX:
								paramBlock[1] = 0x7f; paramBlock[0] = 0xff;
								return true;
								break;
							case USB_AUDIO_REQ_GET_RES:
								paramBlock[1] = 0x00; paramBlock[0] = 0x01;
								return true;
								break;
						}
					}
				}
			}
			break;
		case MIC_FEATURE_NODE:
			if (channelId == 0) {
				if (selector == USB_AUDIO_CSEL_FEATURE_VOLUME) {
					if (paramBlockLength == 2) {
						switch (request) {
							case USB_AUDIO_REQ_GET_CUR:
								paramBlock[1] = (micVolume>>8) & 0xff;
								paramBlock[0] = micVolume & 0xff;
								return true;
								break;
							case USB_AUDIO_REQ_GET_MIN:
								paramBlock[1] = 0x80; paramBlock[0] = 0x00;
								return true;
								break;
							case USB_AUDIO_REQ_GET_MAX:
								paramBlock[1] = 0x7f; paramBlock[0] = 0xff;
								return true;
								break;
							case USB_AUDIO_REQ_GET_RES:
								paramBlock[1] = 0x00; paramBlock[0] = 0x01;
								return true;
								break;
						}
					}
				}
			}
			break;
	}
	return false;
}
	
bool AudioSetControlValue(USB_AUDIO_REQUEST request,
						  uint8_t nodeId,
						  uint8_t channelId,
						  USB_AUDIO_CONTROL_SELECTOR selector,
						  uint8_t* paramBlock,
						  uint8_t paramBlockLength) {
	switch (nodeId) {
		case SPEAKER_FEATURE_NODE:
			if (channelId == 0) {
				if (selector == USB_AUDIO_CSEL_FEATURE_VOLUME) {
					if (paramBlockLength == 2) {
						switch (request) {
							case USB_AUDIO_REQ_SET_CUR:
								*((uint16_t*)(&speakerVolume)) = (paramBlock[1] << 8) | paramBlock[0];
								return true;
						}
					}
				}
			}
			break;
		case MIC_FEATURE_NODE:
			if (channelId == 0) {
				if (selector == USB_AUDIO_CSEL_FEATURE_VOLUME) {
					if (paramBlockLength == 2) {
						switch (request) {
							case USB_AUDIO_REQ_SET_CUR:
								*((uint16_t*)(&micVolume)) = (paramBlock[1] << 8) | paramBlock[0];
								return true;
						}
					}
				}
			}
			break;
		default:
			break;
	}
	return false;
}

bool AudioSetEndpointValue(USB_AUDIO_REQUEST request,
						   uint8_t endpoint,
						   USB_AUDIO_CONTROL_SELECTOR selector,
						   uint8_t* paramBlock,
						   uint8_t paramBlockLength) {
	//we don't do anything useful here - we just allow the host to set the frequency (there's ony one valid one)
	return ((selector == USB_AUDIO_CSEL_ENDPOINT_FREQ) && (paramBlockLength == 3));
}


bool AudioGetEndpointValue(USB_AUDIO_REQUEST request,
						   uint8_t endpoint,
						   USB_AUDIO_CONTROL_SELECTOR selector,
						   uint8_t* paramBlock,
						   uint8_t paramBlockLength) {
	//we don't distinguish requests - we return our sampling rate for min, max, cur and stp
	if ((selector == USB_AUDIO_CSEL_ENDPOINT_FREQ) && (paramBlockLength == 3)) {
		paramBlock[0] = SAMPLE_RATE & 0xff;
		paramBlock[1] = (SAMPLE_RATE >> 8) & 0xff;
		paramBlock[2] = (SAMPLE_RATE >> 16) & 0xff;
		return true;
	} else return false;		//unsupported query
}


USB_Device_Struct device;
bool micStreaming;
bool speakerStreaming;

/** OUT data transferred */
bool USBAudio_ExtendedControlSetupCallback2(USB_Device_Struct* device) {
	if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) != USB_RT_DIR_HOST_TO_DEVICE) return false;
	switch (device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
		case USB_RT_RECIPIENT_INTERFACE:
			//TODO: check wIndexL against control interface *********
			return AudioSetControlValue(device->currentCommand.bRequest,
										device->currentCommand.wIndexH,
										device->currentCommand.wValueL,
										device->currentCommand.wValueH,
										device->commandDataBuffer,
										(device->currentCommand.wLengthH << 8) |
										device->currentCommand.wLengthL);
		case USB_RT_RECIPIENT_ENDPOINT:
			return AudioSetEndpointValue(device->currentCommand.bRequest,
										 device->currentCommand.wIndexL,
										 device->currentCommand.wValueL,
										 device->commandDataBuffer,
										 (device->currentCommand.wLengthH << 8) |
										 device->currentCommand.wLengthL);
	}
	return false;
}
							  

bool USBAudio_ExtendedControlSetupCallback(USB_Device_Struct* device) {
	if ((device->currentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_CLASS) return false;

	bool result = false;
	uint16_t len = (device->currentCommand.wLengthH << 8) | device->currentCommand.wLengthL;

	if ((device->currentCommand.bmRequestType & USB_RT_DIR_MASK) == USB_RT_DIR_HOST_TO_DEVICE) {
		if ((len >= 1) && (len <= USB_MAX_COMMAND_DATA_SIZE)) {
			device->currentCommandDataBase = device->commandDataBuffer;
			device->currentCommandDataRemaining = len;
			device->controlOutDataCompleteCallback = USBAudio_ExtendedControlSetupCallback2;
			result = true;
		}
	} else {
		switch (device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) {
			case USB_RT_RECIPIENT_INTERFACE:
				result = AudioGetControlValue(device->currentCommand.bRequest,
											  device->currentCommand.wIndexH,
											  device->currentCommand.wValueL,
											  device->currentCommand.wValueH,
											  device->commandDataBuffer,
											  len);
				break;
			case USB_RT_RECIPIENT_ENDPOINT:
				result = AudioGetEndpointValue(device->currentCommand.bRequest,
											   device->currentCommand.wIndexL,
											   device->currentCommand.wValueL,
											   device->commandDataBuffer,
											   len);
				break;
		}
		if (result) {
			device->currentCommandDataBase = device->commandDataBuffer;
			device->currentCommandDataRemaining = len;
		}	
	}
	return result;
}

uint32_t counter;

void frameCallback(USB_Device_Struct* device) {
	int16_t buffer[BYTES_PER_FRAME/2];
	uint16_t i;
	
	if (micStreaming) {
		//generate a sawtooth waveform with 1 KHz
		uint16_t outSampleCount = SAMPLE_RATE/1000;
		uint32_t scale = ((GPIO_ReadInput(KEY_PORT, KEY_PIN) ? 0 : (micVolume+0x8000) / (outSampleCount+1)));
		for (i=0;i<outSampleCount;i++) {
			int32_t value = scale * (2*i+1-outSampleCount);
			((int16_t*)(buffer))[i] = S16_TO_LE(value);
		}
		USB_EP_Write(device, 9, (uint8_t*)buffer, outSampleCount*2);
	}
	
	if (speakerStreaming) {
		//read samples coming in and find the value range size = rough estimation of audio level (high frequencies only)
		uint16_t read = USB_EP_Read(device, 8, (uint8_t*)buffer, BYTES_PER_FRAME);
		uint16_t inSampleCount = read / 2;
		int16_t min = 32767;
		int16_t max = -32767;
		for (i=0; i<inSampleCount; i++) {
			int16_t val = buffer[i];
			if (val < min) min = val;
			if (val > max) max = val;
		}
		uint32_t level = (max > min) ? (max - min) : 0;
		level = level * (speakerVolume+0x8000) / 0x10000;	//scale by speaker volume

		//turn audio level into brightness of LED (by very ugly PWM, just 10 levels, but hey, it's just an example)
		level /= 800;
		counter++;
		int phase = counter % 10;
		GPIO_WriteOutput(0,7,phase<level);
	}
}

bool interfaceAltCallback(USB_Device_Struct* device, uint8_t interface, uint8_t value) {
	if (interface == SPEAKER_STREAM_INTERFACE) {
		if (value >= 2) return false;
		speakerStreaming = (value > 0);
		return true;
	} else if (interface == MIC_STREAM_INTERFACE) {
		if (value >= 2) return false;
		micStreaming = (value > 0);
		return true;
	} else return false;
}

void endpointDataCallback(USB_Device_Struct* device, uint8_t epIdx) {
	//Do nothing, just add a handler to prevent pipe from stalling (will stall if no handler is there)
}

void main(void) {
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, false);
	GPIO_SetDir(KEY_PORT, KEY_PIN, GPIO_Input);
	GPIO_SETPULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);

	micStreaming = false;
	speakerStreaming = false;
	micVolume = 0;
	speakerVolume = 0;
	
	device.deviceDescriptor = deviceDescriptor;
	device.configurationCount = 1;
	device.configurationDescriptors[0] = configDescriptor;
	device.stringCount = 4;
	device.strings[0] = languages;
	device.strings[1] = manufacturerName;
	device.strings[2] = deviceName;
	device.strings[3] = serialName;
	device.extendedControlSetupCallback = USBAudio_ExtendedControlSetupCallback;
	device.endpointDataCallback = endpointDataCallback;
	device.frameCallback = frameCallback;
	device.interfaceAltCallback = interfaceAltCallback;
	USBAudio_SetControlValue setControlValueHandler;
	USBAudio_GetControlValue getControlValueHandler;
	USBAudio_SetEndpointValue setEndpointValueHandler;
	USBAudio_GetEndpointValue getEndpointValueHandler;
	
	USB_Init(&device);
	
	USB_SoftConnect(&device);
}