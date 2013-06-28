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

void GPIO_WriteOutput(uint8_t port, uint8_t pin, bool value) {
	GPIO[port].MASKED_DATA[1<<pin] = value ? 0x3fff : 0x0000;
}

bool GPIO_ReadInput(uint8_t port, uint8_t pin) {
	return (GPIO[port].DATA & (1<<pin)) ? true : false;
}

void GPIO_SetPull(HW_RW* pin, IOCON_IO_PULL_MODE mode) {
	*pin = ((*pin) & (~IOCON_IO_PULL_REPEAT)) | mode;
}

void GPIO_SetHysteresis(HW_RW* pin, IOCON_IO_HYSTERESIS_MODE mode) {
	*pin = ((*pin) & (~IOCON_IO_HYSTERESIS_ON)) | mode;
}

void GPIO_SetFunction(HW_RW* pin, IOCON_IO_FUNC func, IOCON_IO_ADMODE admode) {
	*pin = ((*pin) & (~0x87)) | func | admode;
}

void GPIO_SetInterruptMode(uint8_t port, uint8_t pin, GPIO_INTERRUPT_TRIGGER mode) {
	uint32_t mask = 1 << pin;
    if (mode == GPIO_INTERRUPT_NONE) GPIO[port].IE &= ~mask;
    else {
		if ((mode == GPIO_INTERRUPT_HIGH_LEVEL) || (mode == GPIO_INTERRUPT_LOW_LEVEL)) GPIO[port].IS |= mask;
    	else GPIO[port].IS &= ~mask;
    	if (mode == GPIO_INTERRUPT_BOTH_EDGES) GPIO[port].IBE |= mask;
    	else {
    		GPIO[port].IBE &= ~mask;
			if ((mode == GPIO_INTERRUPT_RISING_EDGE) || (mode == GPIO_INTERRUPT_HIGH_LEVEL)) GPIO[port].IEV |= mask;
			else GPIO[port].IEV &= ~mask;
    	}
    	GPIO[port].IE |= mask;
    }
}

uint32_t GPIO_GetInterruptMask(uint8_t port) {
	return GPIO[port].MIS;
}

void GPIO_ClearInterruptMask(uint8_t port, uint32_t mask) {
	GPIO[port].IC = mask;
}
