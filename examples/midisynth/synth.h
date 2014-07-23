//
//  synth.h
//  SamplePlayer
//
//  Created by Matthias Krauß on 13.06.13.
//  Copyright (c) 2013 Matthias Krauß. All rights reserved.
//

#ifndef _SYNTH_
#define _SYNTH_

#include "everykey/everykey.h"

#define NUM_VOICES 8
#define NUM_SYNTHS 4
#define SAMPLE_RATE 31250
#define WAVETABLE_LENGTH_BITS 8 /* size is 1 << WAVETABLE_LENGTH_BITS */

typedef enum {
    Waveform_Rectangle,
    Waveform_Triangle,
    Waveform_Sawtooth,
    Waveform_Sine
} Waveform;

void Synthesizer_Init();
int16_t Synthesizer_GetNextSample();
void Synthesizer_PlayNote(uint8_t voice, uint8_t note, uint8_t velocity);
void Synthesizer_StopNote(uint8_t voice, uint8_t note);

#endif
