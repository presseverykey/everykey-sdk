#include "gpio.h"
#include "memorymap.h"

void GPIO_SetDir(uint8_t port, uint8_t pin, GPIO_Direction dir) {
	switch (dir) {
		case GPIO_Input:
			GPIO[port].DIR &= ~(1<<pin);
			break;
		case GPIO_Output:
			GPIO[port].DIR |= (1<<pin);
			break;
	}
}

void GPIO_SetPull(uint8_t port, uint8_t pin, GPIO_PullState pull) {
	//TODO
}

void GPIO_WriteOutput(uint8_t port, uint8_t pin, bool value) {
	if (value) GPIO[port].MASKED_DATA[1<<pin] = 0x3fff;
	else GPIO[port].MASKED_DATA[1<<pin] = 0x0000;
}

bool GPIO_ReadInput(uint8_t port, uint8_t pin) {
	return (GPIO[port].DATA & (1<<pin)) ? true : false;
}


