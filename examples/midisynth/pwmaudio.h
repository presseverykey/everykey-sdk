#ifndef _PWMAUDIO_
#define _PWMAUDIO_

#include "everykey/everykey.h"

#define PWMAUDIO_TIMER CT16B0
#define PWMAUDIO_PORT 0
#define PWMAUDIO_PIN 10
#define PWMAUDIO_MAT 2

void PWMAudio_Init();

void PWMAudio_SetSample(int16_t sample);

#endif