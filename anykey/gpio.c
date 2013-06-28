#include "gpio.h"
#include "memorymap.h"

void any_gpio_set_dir(any_pin pin, any_direction dir){
	switch (dir) {
		case INPUT:
			GPIO[pin.port].DIR &= ~(1<<pin.pin);
			break;
		case OUTPUT:
			GPIO[pin.port].DIR |= (1<<pin.pin);
			break;
	}
}

void any_gpio_write(any_pin pin, bool value){
	GPIO[pin.port].MASKED_DATA[1<<pin.pin] = value ? 0x3fff : 0x0000;
}

bool any_gpio_read(any_pin pin){
	return (GPIO[pin.port].DATA & (1<<pin.pin)) ? true : false;
}

void any_gpio_set_pull(any_pin pin, any_pull_mode mode){
	*pin.iocon = ((*pin.iocon) & (~REPEAT)) | mode;
}

void any_gpio_set_hysteresis(any_pin pin, any_hysteresis_mode mode){
	*pin.iocon = ((*pin.iocon) & (~HYSTERESIS_ON)) | mode;
}

void any_gpio_set_function(any_pin pin, IOCON_IO_FUNC func){
	*pin.iocon = ((*pin.iocon) & (~0x87)) | func | ADMODE_DIGITAL;
}

void any_gpio_set_analog(any_pin pin) {
	// this one requires a bit of coaxing: the actual flag for
	// setting a function to digitial depends on which pin is
	// being used, some use 0x02, some 0x01 ...
	IOCON_IO_FUNC func;
	switch(pin.port) {
		case 0:
			if (pin.pin == 1) {
				func = IOCON_IO_FUNC_PIO0_11_ADC;
				break;
			} else {
				return; // only pin 1 in port 0 does ADC
			}
		case 1:
			switch(pin.pin) {
				case 0:
				case 1:
				case 2:
				case 3:
					func = 0x02;
					break;
				case 4:
				case 10:
				case 11:
					func = 0x01;
					break;
				default:
					return; // other port 1 pins don't support ADC
			}
			break;
		default:
			return;
	}
	*pin.iocon = ((*pin.iocon) & (~0x87)) | func | ADMODE_DIGITAL;
}

void any_gpio_set_interrupt_mode(any_pin pin, any_interrupt_mode mode){
	uint32_t mask = 1 << pin.pin;
	if (mode == INTERRUPT_NONE) {
		GPIO[pin.port].IE &= ~mask;
	} else {
		if ((mode == INTERRUPT_HIGH_LEVEL) || (mode == INTERRUPT_LOW_LEVEL)){
			GPIO[pin.port].IS |= mask;
		} else {
			GPIO[pin.port].IS &= ~mask;
		}
		if (mode == INTERRUPT_BOTH_EDGES) {
			GPIO[pin.port].IBE |= mask;
		} else {
			GPIO[pin.port].IBE &= ~mask;
			if ((mode == INTERRUPT_RISING_EDGE) || (mode == INTERRUPT_HIGH_LEVEL)) {
				GPIO[pin.port].IEV |= mask;
			} else {
				GPIO[pin.port].IEV &= ~mask;
			}
		}
		GPIO[pin.port].IE |= mask;
	}
}

uint32_t any_gpio_get_interrupt_mask(uint8_t port) {
	return GPIO[port].MIS;
}

void any_gpio_clear_interrupt_mask(uint8_t port, uint32_t mask) {
	GPIO[port].IC = mask;
}
