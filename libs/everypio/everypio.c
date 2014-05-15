#include "everypio.h"



static void _everypio_set_digital_pio(every_pin pin) {
	IOCON_IO_FUNC func = 0x00;
	if (pin.port == 0 && (pin.pin == 10 || pin.pin == 11 || pin.pin == 0) ) {
		func = 0x01;
	}
	if (pin.port == 1 && (pin.pin < 4) ) {
		func = 0x01;
	}
	every_gpio_set_function(pin.iocon, func, IOCON_IO_ADMODE_DIGITAL);
}
void everypio_write(every_pin pin, bool val){
	_everypio_set_digital_pio(pin);
	every_gpio_set_dir(pin.port, pin.pin, OUTPUT);
	every_gpio_write(pin.port, pin.pin, val);
}

void everypio_led(bool on) {
	everypio_write(LED, on);
}

void everypio_led_toggle() {
	everypio_led(!everypio_read(LED));
}

void everypio_digital_input_set(every_pin pin, every_gpio_pull_mode mode) {
	_everypio_set_digital_pio(pin);
	every_gpio_set_dir(pin.port, pin.pin, INPUT);
	every_gpio_set_pull(pin.iocon, mode);

}

bool everypio_read(every_pin pin){
	return every_gpio_read(pin.port, pin.pin);
}

static inline uint8_t _everypio_map_pin_to_channel (every_pin pin) {
	switch((int)pin.iocon) {
		case (int)&IOCON->PIO0_11:
			return 0;
			break;
		case (int)&IOCON->PIO1_0:
			return 1;
			break;
		case (int)&IOCON->PIO1_1:
			return 2;
			break;
		case (int)&IOCON->PIO1_2:
			return 3;
			break;
		case (int)&IOCON->PIO1_3:
			return 4;
			break;
		case (int)&IOCON->PIO1_4:
			return 5;
			break;
		case (int)&IOCON->PIO1_10:
			return 6;
			break;
		case (int)&IOCON->PIO1_11:
			return 7;
			break;
    default:
      return 8;
	}
}
static uint8_t everypio_analog_config = 0;
void everypio_analog_input_set(every_pin pin, bool on_off) {
	uint8_t channel = _everypio_map_pin_to_channel(pin);
	if (channel == 8) {
		return;
	}
	if (on_off && everypio_analog_config == 0) {
		ADC_Init();
	}
	if (on_off) {
		everypio_analog_config |= (1 << channel);
		every_gpio_set_function(pin.iocon, (channel < 5 ? 0x02 : 0x01), IOCON_IO_ADMODE_ANALOG);
		every_gpio_set_dir(pin.port, pin.pin, INPUT);
	} else {
		everypio_analog_config &= ~(1 << channel);
	}
	if (!on_off && everypio_analog_config == 0) {
		ADC_Disable();
	}
}
int everypio_analog_read(every_pin pin){
	// check this is an adc pin
	// 8 pins support adc, they are mapped to "channels" 0 - 7, lookup
	// channel
	uint8_t channel = _everypio_map_pin_to_channel(pin);
	if (channel == 8) {
		return -1;
	}
	if (everypio_analog_config & ( 1 << channel)) {
		return ADC_Read(channel);
	} else {
		return -1;
	}
}
