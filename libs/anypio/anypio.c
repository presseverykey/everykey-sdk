#include "anypio.h"


static void _anypio_set_digital_pio(any_pin pin) {
	IOCON_IO_FUNC func = 0x00;
	if (pin.port == 0 && (pin.pin == 10 || pin.pin == 11) ) {
		func = 0x01;
	}
	if (pin.port == 1 && (pin.pin < 4) ) {
		func = 0x01;
	}
	any_gpio_set_function(pin.iocon, func, IOCON_IO_ADMODE_DIGITAL);
}
void anypio_write(any_pin pin, bool val){
	_anypio_set_digital_pio(pin);
	any_gpio_set_dir(pin.port, pin.pin, OUTPUT);
	any_gpio_write(pin.port, pin.pin, val);
}

bool anypio_read(any_pin pin, any_gpio_pull_mode mode){
	_anypio_set_digital_pio(pin);
	any_gpio_set_dir(pin.port, pin.pin, INPUT);
	any_gpio_set_pull(pin.iocon, mode);
	return any_gpio_read(pin.port, pin.pin);
}

static uint8_t anypio_analog_config = 0;
void anypio_analog_set(any_pin pin, bool on_off) {
	uint8_t channel;
	ANY_PIO_MAP_CHANNEL(pin, channel);
	if (channel == 8) {
		return;
	}
	if (on_off && anypio_analog_config == 0) {
		ADC_Init();
	}
	if (on_off) {
		anypio_analog_config |= (1 << channel);
		any_gpio_set_function(pin.iocon, (channel < 5 ? 0x02 : 0x01), IOCON_IO_ADMODE_ANALOG);
		any_gpio_set_dir(pin.port, pin.pin, INPUT);
	} else {
		anypio_analog_config &= ~(1 << channel);
	}
	if (!on_off && anypio_analog_config == 0) {
		ADC_Disable();
	}
}
int anypio_analog_read(any_pin pin){
	// check this is an adc pin
	// 8 pins support adc, they are mapped to "channels" 0 - 7, lookup
	// channel
	uint8_t channel;
	ANY_PIO_MAP_CHANNEL(pin, channel);
	if (channel == 8) {
		return -1;
	}
	if (anypio_analog_config & ( 1 << channel)) {
		return ADC_Read(channel);
	} else {
		return -1;
	}
}
