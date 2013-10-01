#include "midi.h"

void USBMIDI_Send(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* midi);

bool USBMIDI_EnqueueBlock(const USBMIDI_Behaviour_Struct* behaviour, uint8_t* block) {
	uint16_t wrIdx = *(behaviour->cmdFifoWrIdx);
	uint16_t nextWrIdx = (wrIdx + 4) % (behaviour->cmdFifoSize);
	if (nextWrIdx == *(behaviour->cmdFifoRdIdx)) return false;	//Would be full
	behaviour->cmdFifo[wrIdx  ] = block[0];
	behaviour->cmdFifo[wrIdx+1] = block[1];
	behaviour->cmdFifo[wrIdx+2] = block[2];
	behaviour->cmdFifo[wrIdx+3] = block[3];
	*(behaviour->cmdFifoWrIdx) = nextWrIdx;
	return true;
}

bool USBMIDI_SendNoteOn(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t note, uint8_t velocity) {
	uint8_t block[4];
	block[0] = ((cableNumber & 0x0f) << 4) | 0x09;
	block[1] = 0x90 | (channel & 0x0f);
	block[2] = note & 0x7f;
	block[3] = velocity & 0x7f;
	return USBMIDI_EnqueueBlock(behaviour, block);
}

bool USBMIDI_SendNoteOff(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t note) {
	uint8_t block[4];
	block[0] = ((cableNumber & 0x0f) << 4) | 0x09;
	block[1] = 0x90 | (channel & 0x0f);
	block[2] = note & 0x7f;
	return USBMIDI_EnqueueBlock(behaviour, block);
}

bool USBMIDI_SendControlChange(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t channel, uint8_t control, uint8_t value) {
	uint8_t block[4];
	block[0] = ((cableNumber & 0x0f) << 4) | 0x0B;
	block[1] = 0xB0 | (channel & 0x0f);
	block[2] = control & 0x7f;
	block[3] = value & 0x7f;
	return USBMIDI_EnqueueBlock(behaviour, block);
}

bool USBMIDI_SendSysEx(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* behaviour, uint8_t cableNumber, uint8_t* data, uint8_t len) {
	uint8_t block[4];
	uint8_t written = 0;
	while (written < len) {
		switch (len - written) {
			case 1:
				block[0] = ((cableNumber & 0x0f) << 4) | 0x05;
				block[1] = data[written];
				if (!USBMIDI_EnqueueBlock(behaviour, block)) return false;
				written += 1;
				break;
			case 2:
				block[0] = ((cableNumber & 0x0f) << 4) | 0x06;
				block[1] = data[written];
				block[2] = data[written+1];
				if (!USBMIDI_EnqueueBlock(behaviour, block)) return false;
				written += 2;
				break;
			case 3:
				block[0] = ((cableNumber & 0x0f) << 4) | 0x07;
				block[1] = data[written];
				block[2] = data[written+1];
				block[3] = data[written+2];
				if (!USBMIDI_EnqueueBlock(behaviour, block)) return false;
				written += 3;
				break;
			default:
				block[0] = ((cableNumber & 0x0f) << 4) | 0x04;
				block[1] = data[written];
				block[2] = data[written+1];
				block[3] = data[written+2];
				if (!USBMIDI_EnqueueBlock(behaviour, block)) return false;
				written += 3;
				break;
		}
	}
	return true;
}


/** called when the data from a set command has arrived */
bool USBMIDI_SetReportDataComplete(USB_Device_Struct* device) {

	return false;	//TODO
}

/** handler for MIDI-specific USB commands */
bool USBMIDI_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {

	// we only handle requests to our interface	
	if ((device->currentCommand.bmRequestType & USB_RT_RECIPIENT_MASK) != USB_RT_RECIPIENT_INTERFACE) return false;

	const USBMIDI_Behaviour_Struct* midi = (const USBMIDI_Behaviour_Struct*)behaviour;
	
	if (device->currentCommand.wIndexL != midi->interfaceNumber) return false;

	uint16_t maxTransferLen = (device->currentCommand.wLengthH << 8) | device->currentCommand.wLengthL;

	//MIDI does not modify standard requests, so we can skip to class-specific requests
	
	//From now, only handle class-specific requests
	if ((device->currentCommand.bmRequestType & USB_RT_TYPE_MASK) != USB_RT_TYPE_CLASS) return false;
	
	switch (device->currentCommand.bRequest) {
		//TODO ********
		return false;
	}
	return false;
}

bool USBMIDI_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx) {
	const USBMIDI_Behaviour_Struct* midi = (const USBMIDI_Behaviour_Struct*)behaviour;
	if ((midi->inBuffer) && (epIdx == midi->inDataEndpoint)) {
		USBMIDI_Send(device,(const USBMIDI_Behaviour_Struct*)behaviour);
		return true;
	}
	if ((midi->outBuffer) && (epIdx == midi->outDataEndpoint)) {	//Data from host - parse
		uint16_t transfer = USB_EP_Read(device, midi->outDataEndpoint, midi->outBuffer, USB_MAX_BULK_DATA_SIZE);
		transfer &= 0xfc;	//Round down to 4-byte blocks
		uint16_t off;
		for (off=0; off<transfer; off+=4) {
			uint8_t cableNumber = (midi->outBuffer[off] >> 4) & 0x0f;
			uint8_t codeIndexNumber = midi->outBuffer[off] & 0x0f;
			switch (codeIndexNumber) {
				case 0x04:	//SysEx start/continue
					if (midi->sysExHandler) midi->sysExHandler(device, midi, cableNumber, &(midi->outBuffer[off+1]), 3, true);
					break;
				case 0x05:	//SysEx end 1 byte (TODO: Could also be 1 byte system control message **************)
					if (midi->sysExHandler) midi->sysExHandler(device, midi, cableNumber, &(midi->outBuffer[off+1]), 1, false);
					break;
				case 0x06:	//SysEx end 2 byte
					if (midi->sysExHandler) midi->sysExHandler(device, midi, cableNumber, &(midi->outBuffer[off+1]), 2, false);
					break;
				case 0x07:	//SysEx end 3 byte
					if (midi->sysExHandler) midi->sysExHandler(device, midi, cableNumber, &(midi->outBuffer[off+1]), 3, false);
					break;
				case 0x08:	//Note off
					if (midi->noteOffHandler) midi->noteOffHandler(device, midi, cableNumber, (midi->outBuffer[off+1])&0x0f, midi->outBuffer[off+2]);
					break;
				case 0x09:	//Note on
					if (midi->noteOnHandler) midi->noteOnHandler(device, midi, cableNumber, (midi->outBuffer[off+1])&0x0f, midi->outBuffer[off+2], midi->outBuffer[off+3]);
					break;
				case 0x0B:	//Control Change
					if (midi->controlChangeHandler) midi->controlChangeHandler(device, midi, cableNumber, (midi->outBuffer[off+1])&0x0f, midi->outBuffer[off+2], midi->outBuffer[off+3]);
					break;
			}

		}
		return true;
	}
	return false;
}

/** we use this callback to reset our protocol and idle values */
void USBMIDI_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	const USBMIDI_Behaviour_Struct* midi = (const USBMIDI_Behaviour_Struct*)behaviour;
	*(midi->cmdFifoRdIdx) = 0;
	*(midi->cmdFifoWrIdx) = 0;
}

void USBMIDI_FrameHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	USBMIDI_Send(device,(const USBMIDI_Behaviour_Struct*)behaviour);
}

void USBMIDI_Send(USB_Device_Struct* device, const USBMIDI_Behaviour_Struct* midi) {
	if (USB_EP_GetFull(device, midi->inDataEndpoint)) return;
	uint16_t written = 0;

	while ((written < USB_MAX_BULK_DATA_SIZE) && (*(midi->cmdFifoRdIdx) != *(midi->cmdFifoWrIdx))) {
		uint16_t rdIdx = *(midi->cmdFifoRdIdx);
		midi->inBuffer[written] = midi->cmdFifo[rdIdx];
		midi->inBuffer[written+1] = midi->cmdFifo[rdIdx+1];
		midi->inBuffer[written+2] = midi->cmdFifo[rdIdx+2];
		midi->inBuffer[written+3] = midi->cmdFifo[rdIdx+3];
		written += 4;
		*(midi->cmdFifoRdIdx) = (rdIdx + 4) % midi->cmdFifoSize;
	}
	if (written > 0) {
		USB_EP_Write(device, midi->inDataEndpoint, midi->inBuffer, written);
		//TODO: What to do if not all data could be written? Right now, we assume everything works fine ********************
	}


}

