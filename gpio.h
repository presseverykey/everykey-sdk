#ifndef _GPIO_
#define _GPIO_

#include "types.h"

typedef enum GPIO_Direction {
	GPIO_Input = 0,
	GPIO_Output
} GPIO_Direction;

typedef enum GPIO_PullState {
	GPIO_PullUp = 0,
	GPIO_PullDown,
	GPIO_Repeat,
	GPIO_None
} GPIO_PullState;

void GPIO_SetDir(uint8_t port, uint8_t pin, GPIO_Direction dir);
void GPIO_SetPull(uint8_t port, uint8_t pin, GPIO_PullState pull);
void GPIO_WriteOutput(uint8_t port, uint8_t pin, bool value);
bool GPIO_ReadInput(uint8_t port, uint8_t pin);

#endif
