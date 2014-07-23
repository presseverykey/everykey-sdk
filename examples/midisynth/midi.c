#include "everykey/everykey.h"
#include "midi.h"

uint8_t midiOutBuffer[MIDIOUT_BUFFER_LENGTH];
uint8_t midiOutReadIdx = 0;
uint8_t midiOutWriteIdx = 0;
uint8_t midiOutState = 0;

uint8_t midiInState = 0;	//0 = wait for start bit, 1 = read start bit, 2-9: read bits
uint8_t midiInAccumulator;

uint8_t midiParseState = 0; //0 = waiting for command, 1.. = read data i-1
uint8_t midiParseCommand;
uint8_t midiParseDataLength;
uint8_t midiParseBuffer[MIDIPARSE_BUFFER_LENGTH];

MIDI_NoteHandler noteHandler = NULL;
MIDI_TickHandler tickHandler = NULL;

void MIDI_WriteCommand(const uint8_t* buffer, uint8_t len) {
	uint8_t i;
	for (i=0; i<len; i++) {
		midiOutBuffer[midiOutWriteIdx] = buffer[i];
		midiOutWriteIdx = (midiOutWriteIdx+1) & (MIDIOUT_BUFFER_LENGTH-1);
	}
}

void MIDI_WriteNote(uint8_t channel, uint8_t note, uint8_t velocity) {
	uint8_t buffer[] = { 144+channel, note, velocity };
	MIDI_WriteCommand(buffer, 3);
}

void MIDI_ParseByte(uint8_t byte) {
	if (byte & 0x80) {
		switch (byte & 0xf0) {
			case 0x80:	//note off
			case 0x90:	//note on
				midiParseCommand = byte;
				midiParseDataLength = 2;	//Two bytes to read: 1. note, 2. velocity
				midiParseState = 1;
				break;
			default:
				midiParseState = 0;
				break;
		}
	} else {
		midiParseBuffer[midiParseState-1] = byte;
		midiParseState++;
		if (midiParseState > midiParseDataLength) {	//command complete
			midiParseState = 1;		//running status
			switch (midiParseCommand & 0xf0) {
				case 0x80:
				 	if (noteHandler) (*noteHandler)(midiParseCommand & 0x0f,midiParseBuffer[0],0);
					break;
				case 0x90:
					if (noteHandler) (*noteHandler)(midiParseCommand & 0x0f,midiParseBuffer[0],midiParseBuffer[1]);
					break;
			}
		}
	}
}

void MIDI_Init(MIDI_NoteHandler inNoteHandler, MIDI_TickHandler inTickHandler) {
	noteHandler = inNoteHandler;
	tickHandler = inTickHandler;

	every_gpio_set_dir(MIDIOUT_PORT, MIDIOUT_PIN, OUTPUT);
	every_gpio_write(MIDIOUT_PORT, MIDIOUT_PIN, 1);	//Idle state
	// TODO: Set I2Cplus on midiOut to enable high current drain?

	every_gpio_set_dir(MIDIIN_PORT, MIDIIN_PIN, INPUT);
	EVERY_GPIO_SETHYSTERESIS(MIDIIN_PORT, MIDIIN_PIN, HYSTERESIS_ON);

	Timer_Enable(CT16B1, true);
	Timer_SetPrescale(CT16B1, 0);					//Use full clock speed
	Timer_SetMatchValue(CT16B1, 0, MIDI_CLOCK/2);	//We use MAT0 for reading
	Timer_SetMatchBehaviour(CT16B1, 0, 0);			//Currently off
	Timer_SetMatchValue(CT16B1, 1, MIDI_CLOCK);		//We use MAT1 for writing
	Timer_SetMatchBehaviour(CT16B1, 1, TIMER_MATCH_INTERRUPT | TIMER_MATCH_RESET);
	NVIC_EnableInterrupt(NVIC_CT16B1);
	Timer_Start(CT16B1);

	//Turn on start bit detection
	NVIC_EnableInterrupt(NVIC_PIO_2);
	every_gpio_set_interrupt_mode(MIDIIN_PORT, MIDIIN_PIN, TRIGGER_FALLING_EDGE);
}

//Start of a MIDI IN byte transfer
void gpio2_handler () {
	uint32_t mask = every_gpio_get_interrupt_mask(MIDIIN_PORT);

	//Turn off start bit detection
	every_gpio_set_interrupt_mode(MIDIIN_PORT, MIDIIN_PIN, TRIGGER_NONE);

	//init byte reading
	if (midiInState == 0) {
		midiInState = 1;
		midiInAccumulator = 0;
	}

	//Turn on timer input sampling - close to half cycle
	uint32_t counterVal = (Timer_GetValue(CT16B1) + 1150) % MIDI_CLOCK;
	Timer_SetMatchValue(CT16B1, 0, counterVal);
	Timer_SetMatchBehaviour(CT16B1, 0, TIMER_MATCH_INTERRUPT);

	//Interrupt is handled
	every_gpio_clear_interrupt_mask(MIDIIN_PORT, mask);
}

//timer handler: read / write bytes
void ct16b1_handler() {
	uint32_t intMask = Timer_GetInterruptMask(CT16B1);
	Timer_ClearInterruptMask(CT16B1, intMask);
	bool read = intMask & TIMER_MR0INT;
	bool write = intMask & TIMER_MR1INT;

	if (read) {
		if (midiInState > 0) {
			if (midiInState > 1) {
				uint8_t bit = every_gpio_read(MIDIIN_PORT,MIDIIN_PIN) ? 1 : 0;
				midiInAccumulator |= bit << (midiInState-2);
			}
			midiInState++;
			if (midiInState > 9) {	//byte done

				//Go back from timer read to start bit detection
				Timer_SetMatchBehaviour(CT16B1, 0, 0);
				midiInState = 0;
				every_gpio_clear_interrupt_mask(MIDIIN_PORT, 1 << MIDIIN_PIN);
				every_gpio_set_interrupt_mode(MIDIIN_PORT, MIDIIN_PIN, TRIGGER_FALLING_EDGE);

				//Handle received byte
				MIDI_ParseByte(midiInAccumulator);

			}
		}
	}

	if (write) {
		switch (midiOutState) {
			case 0:	//Idle state: Check if there's something to write, trigger start bit if necessary
	            if (midiOutReadIdx != midiOutWriteIdx) {
                	every_gpio_write(MIDIOUT_PORT, MIDIOUT_PIN, 0);
                	midiOutState++;
            	}
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				every_gpio_write(MIDIOUT_PORT, MIDIOUT_PIN, midiOutBuffer[midiOutReadIdx] & (0x01 << (midiOutState-1)));
				midiOutState++;
				break;
			case 9:	//Data has been sent. Send stop bit, increment read index, goto idle
				every_gpio_write(MIDIOUT_PORT, MIDIOUT_PIN, 1);
				midiOutReadIdx = (midiOutReadIdx+1) & (MIDIOUT_BUFFER_LENGTH-1);
            	midiOutState = 0;
            	break;
		}
	}

	//Callback if needed
	if (tickHandler) (*tickHandler)();

}
