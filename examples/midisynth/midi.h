
#ifndef _MIDI_
#define _MIDI_

#define MIDIOUT_PORT 0
#define MIDIOUT_PIN 4
#define MIDIIN_PORT 2
#define MIDIIN_PIN 2
#define MIDI_CLOCK 2303 /* 2303 = 72000 / 31250 */

#define MIDIOUT_BUFFER_LENGTH 128 /* must be POT */
#define MIDIPARSE_BUFFER_LENGTH 4

void MIDI_WriteNote(uint8_t channel, uint8_t note, uint8_t velocity);
void MIDI_WriteCommand(const uint8_t* buffer, uint8_t len);

typedef void (*MIDI_NoteHandler)(uint8_t channel, uint8_t note, uint8_t velocity);
typedef void (*MIDI_TickHandler)(void);

void MIDI_Init(MIDI_NoteHandler noteHandler, MIDI_TickHandler tickHandler);

#endif


