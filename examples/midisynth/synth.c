//
//  synth.c
//  SamplePlayer
//
//  Created by Matthias Krauß on 13.06.13.
//  Copyright (c) 2013 Matthias Krauß. All rights reserved.
//

#include "synth.h"

const uint16_t freqLookup[128] = {17, 18, 19, 20, 22, 23, 24, 26, 27, 29, 31, 32, 34, 36, 38, 41, 43, 46, 48, 51, 54, 58, 61, 65, 69, 73, 77, 82, 86, 92, 97, 103, 109, 115, 122, 129, 137, 145, 154, 163, 173, 183, 194, 206, 218, 231, 244, 259, 274, 291, 308, 326, 346, 366, 388, 411, 435, 461, 489, 518, 549, 581, 616, 652, 691, 732, 776, 822, 871, 923, 978, 1036, 1097, 1163, 1232, 1305, 1383, 1465, 1552, 1644, 1742, 1845, 1955, 2071, 2195, 2325, 2463, 2610, 2765, 2930, 3104, 3288, 3484, 3691, 3910, 4143, 4389, 4650, 4927, 5220, 5530, 5859, 6207, 6577, 6968, 7382, 7821, 8286, 8779, 9301, 9854, 10440, 11060, 11718, 12415, 13153, 13935, 14764, 15642, 16572, 17557, 18601, 19708, 20879, 22121, 23436, 24830, 26306};
const int16_t sinewave[256] = {0, 201, 402, 603, 803, 1003, 1202, 1401, 1598, 1795, 1990, 2185, 2378, 2570, 2760, 2948, 3135, 3320, 3503, 3683, 3862, 4038, 4212, 4383, 4551, 4717, 4880, 5040, 5197, 5351, 5501, 5649, 5793, 5933, 6070, 6203, 6333, 6458, 6580, 6698, 6811, 6921, 7027, 7128, 7225, 7317, 7405, 7489, 7568, 7643, 7713, 7779, 7839, 7895, 7946, 7993, 8035, 8071, 8103, 8130, 8153, 8170, 8182, 8190, 8192, 8190, 8182, 8170, 8153, 8130, 8103, 8071, 8035, 7993, 7946, 7895, 7839, 7779, 7713, 7643, 7568, 7489, 7405, 7317, 7225, 7128, 7027, 6921, 6811, 6698, 6580, 6458, 6333, 6203, 6070, 5933, 5793, 5649, 5501, 5351, 5197, 5040, 4880, 4717, 4551, 4383, 4212, 4038, 3862, 3683, 3503, 3320, 3135, 2948, 2760, 2570, 2378, 2185, 1990, 1795, 1598, 1401, 1202, 1003, 803, 603, 402, 201, 0, -201, -402, -603, -803, -1003, -1202, -1401, -1598, -1795, -1990, -2185, -2378, -2570, -2760, -2948, -3135, -3320, -3503, -3683, -3862, -4038, -4212, -4383, -4551, -4717, -4880, -5040, -5197, -5351, -5501, -5649, -5793, -5933, -6070, -6203, -6333, -6458, -6580, -6698, -6811, -6921, -7027, -7128, -7225, -7317, -7405, -7489, -7568, -7643, -7713, -7779, -7839, -7895, -7946, -7993, -8035, -8071, -8103, -8130, -8153, -8170, -8182, -8190, -8192, -8190, -8182, -8170, -8153, -8130, -8103, -8071, -8035, -7993, -7946, -7895, -7839, -7779, -7713, -7643, -7568, -7489, -7405, -7317, -7225, -7128, -7027, -6921, -6811, -6698, -6580, -6458, -6333, -6203, -6070, -5933, -5793, -5649, -5501, -5351, -5197, -5040, -4880, -4717, -4551, -4383, -4212, -4038, -3862, -3683, -3503, -3320, -3135, -2948, -2760, -2570, -2378, -2185, -1990, -1795, -1598, -1401, -1202, -1003, -803, -603, -402, -201};

typedef struct {
    Waveform waveform;
} Voice;

typedef struct {
    uint32_t phase;
    uint16_t freq;
    uint8_t voice;
    uint8_t note;
    uint8_t velocity;   //0 = not playing
} Synth;

Voice voices[NUM_VOICES];
Synth synths[NUM_SYNTHS];
uint8_t nextSynth = 0;

void Synthesizer_Init() {
    voices[0].waveform = Waveform_Rectangle;
    voices[1].waveform = Waveform_Triangle;
    voices[2].waveform = Waveform_Sawtooth;
    voices[3].waveform = Waveform_Sine;
    uint8_t s;
    for (s = 0; s < NUM_SYNTHS; s++) {
        synths[s].velocity = 0;
    }
}
int16_t Synth_GetNextSample(Synth* synth) {
    if (!(synth->velocity)) return 0;
    synth->phase += synth->freq;
    uint16_t idx = synth->phase & 0xffff;
    switch (voices[synth->voice].waveform) {
        case Waveform_Rectangle:
            return (idx & 0x8000) ? 0x1000 : -0x1000;
            break;
        case Waveform_Sawtooth:
            return -0x1000 + (idx >> 3);
            break;
        case Waveform_Triangle:
            return (idx & 0x8000) ? (0x3000 - (idx >> 2)) : (-0x1000 + (idx >> 2));
            break;
        case Waveform_Sine:
            return sinewave[idx >> (16-WAVETABLE_LENGTH_BITS)];
            break;
    }
    //Should not reach this point
    return 0;
}

int16_t Synthesizer_GetNextSample() {
    int16_t sum = 0;
    uint8_t s;
    for (s = 0; s < NUM_SYNTHS; s++) {
        sum += Synth_GetNextSample(&(synths[s]));
    }
    return sum;
}

void Synthesizer_PlayNote(uint8_t voice, uint8_t note, uint8_t velocity) {
    uint8_t s;
    for (s = 0; s < NUM_SYNTHS; s++) {
        if (!(synths[s].velocity)) {
            nextSynth = s;
            break;
        }
    }
    synths[nextSynth].phase = 0;
    synths[nextSynth].note = note;
    synths[nextSynth].freq = freqLookup[note];
    synths[nextSynth].velocity = velocity;
    synths[nextSynth].voice = voice;
    nextSynth = (nextSynth+1) % NUM_SYNTHS;
}

void Synthesizer_StopNote(uint8_t voice, uint8_t note) {
    uint8_t s;
    for (s = 0; s < NUM_SYNTHS; s++) {
        if (synths[s].voice != voice) continue;
        if (synths[s].note != note) continue;
        synths[s].velocity = 0;
    }
}
