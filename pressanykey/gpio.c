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

void GPIO_SetFunction(HW_RW* pin, IOCON_IO_FUNC func) {
	*pin = ((*pin) & (~0x07)      ) | func;
  *pin = ((*pin) & (~ (0x01<<7))) | (IOCON_IO_ADMODE_ANALOG << 7);
}
