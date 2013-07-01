#include "gpio.h"
#include "memorymap.h"

void any_gpio_set_dir(uint8_t port, uint8_t pin, any_gpio_direction dir) {
	switch (dir) {
		case INPUT:
			GPIO[port].DIR &= ~(1<<pin);
			break;
		case OUTPUT:
			GPIO[port].DIR |= (1<<pin);
			break;
	}
}

void any_gpio_write(uint8_t port, uint8_t pin, bool value) {
	GPIO[port].MASKED_DATA[1<<pin] = value ? 0x3fff : 0x0000;
}

bool any_gpio_read(uint8_t port, uint8_t pin) {
	return (GPIO[port].DATA & (1<<pin)) ? true : false;
}

void any_gpio_set_pull(HW_RW* pin, any_gpio_pull_mode mode) {
	*pin = ((*pin) & (~REPEAT)) | mode;
}

void any_gpio_set_hysteresis(HW_RW* pin, any_gpio_hysteresis_mode mode) {
	*pin = ((*pin) & (~HYSTERESIS_ON)) | mode;
}

void any_gpio_set_function(HW_RW* pin, IOCON_IO_FUNC func, IOCON_IO_ADMODE admode) {
	*pin = ((*pin) & (~0x87)) | func | admode;
}

void any_gpio_set_interrupt_mode(uint8_t port, uint8_t pin, any_gpio_interrupt_mode mode) {
	uint32_t mask = 1 << pin;
    if (mode == TRIGGER_NONE){
			GPIO[port].IE &= ~mask;
    } else {
			if ((mode == TRIGGER_HIGH_LEVEL) || (mode == TRIGGER_LOW_LEVEL)){
				GPIO[port].IS |= mask;
			} else {
				GPIO[port].IS &= ~mask;
			}
			if (mode == TRIGGER_BOTH_EDGES){
				GPIO[port].IBE |= mask;
			} else {
				GPIO[port].IBE &= ~mask;
				if ((mode == TRIGGER_RISING_EDGE) || (mode == TRIGGER_HIGH_LEVEL)){
					GPIO[port].IEV |= mask;
				} else{
					GPIO[port].IEV &= ~mask;
				}
			}

    	GPIO[port].IE |= mask;
    }
}

uint32_t any_gpio_get_interrupt_mask(uint8_t port) {
	return GPIO[port].MIS;
}

void any_gpio_clear_interrupt_mask(uint8_t port, uint32_t mask) {
	GPIO[port].IC = mask;
}
