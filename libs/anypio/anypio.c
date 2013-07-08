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

static bool led = true;
void anypio_led(bool on) {
	led = on;
	anypio_write(LED, on);
}

void anypio_led_toggle() {
	anypio_led(!led);
}

void anypio_digital_input_set(any_pin pin, any_gpio_pull_mode mode) {
	_anypio_set_digital_pio(pin);
	any_gpio_set_dir(pin.port, pin.pin, INPUT);
	any_gpio_set_pull(pin.iocon, mode);

}

bool anypio_read(any_pin pin){
	return any_gpio_read(pin.port, pin.pin);
}

static inline uint8_t _anypio_map_pin_to_channel (any_pin pin) {
	switch((int)pin.iocon) {
		case (int)&IOCON->PIO0_10:
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
static uint8_t anypio_analog_config = 0;
void anypio_analog_input_set(any_pin pin, bool on_off) {
	uint8_t channel = _anypio_map_pin_to_channel(pin);
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
	uint8_t channel = _anypio_map_pin_to_channel(pin);
	if (channel == 8) {
		return -1;
	}
	if (anypio_analog_config & ( 1 << channel)) {
		return ADC_Read(channel);
	} else {
		return -1;
	}
}
