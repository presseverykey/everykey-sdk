#ifndef EVERYPIO_H
#define EVERYPIO_H

#include "everykey/everykey.h"




typedef struct every_pin {
  uint8_t port;
  uint8_t pin;
  HW_RW   *iocon;
} every_pin;
#define LED       (every_pin){0,7, &IOCON->PIO0_7}
#define KEY_REV1  (every_pin){1,4, &IOCON->PIO1_4}
#define KEY1_REV2 (every_pin){0,1, &IOCON->PIO0_1}
#define KEY2_REV2 (every_pin){0,0, &IOCON->PIO0_0}
#define PIN_0_0   (every_pin){0,0, &IOCON->PIO0_0}
#define PIN_0_1   (every_pin){0,1, &IOCON->PIO0_1}
#define PIN_0_2   (every_pin){0,2, &IOCON->PIO0_2}
#define PIN_0_4   (every_pin){0,4, &IOCON->PIO0_4}
#define PIN_0_5   (every_pin){0,5, &IOCON->PIO0_5}
#define PIN_0_6   (every_pin){0,6, &IOCON->PIO0_6}
#define PIN_0_7   (every_pin){0,7, &IOCON->PIO0_7}
#define PIN_0_8   (every_pin){0,8, &IOCON->PIO0_8}
#define PIN_0_9   (every_pin){0,9, &IOCON->PIO0_9}
#define PIN_0_10  (every_pin){0,10,&IOCON->PIO0_10}
#define PIN_0_11  (every_pin){0,11,&IOCON->PIO0_11}
#define PIN_1_0   (every_pin){1,0, &IOCON->PIO1_0}
#define PIN_1_1   (every_pin){1,1, &IOCON->PIO1_1}
#define PIN_1_2   (every_pin){1,2, &IOCON->PIO1_2}
#define PIN_1_3   (every_pin){1,3, &IOCON->PIO1_3}
#define PIN_1_4   (every_pin){1,4, &IOCON->PIO1_4}
#define PIN_1_5   (every_pin){1,5, &IOCON->PIO1_5}
#define PIN_1_6   (every_pin){1,6, &IOCON->PIO1_6}
#define PIN_1_7   (every_pin){1,7, &IOCON->PIO1_7}
#define PIN_2_0   (every_pin){2,0, &IOCON->PIO2_0}
#define PIN_2_1   (every_pin){2,1, &IOCON->PIO2_1}
#define PIN_2_11  (every_pin){2,11,&IOCON->PIO2_11}
#define PIN_2_2   (every_pin){2,2, &IOCON->PIO2_2}
#define PIN_2_3   (every_pin){2,3, &IOCON->PIO2_3}


// turns the indicated pin on or off.
void everypio_write(every_pin, bool);

// turn led on or off
void everypio_led(bool);

// toggle led
void everypio_led_toggle();

// reads the value set at the indicated pin.
// Before calling this function, pins need to be
// configured as INPUT by calling `everypio_digital_input_set`
// see below.
bool everypio_read(every_pin);


// configures the given pin as a digital input pin,
// the pin needs to be configured for input before
// using `everypio_gpio_read`.
// Along with the pin to configure, it's necessary to
// provide a value of the pull_up/down mode. 
//
// Possible values are:
//    NONE
//    PULL_DOWN
//    PULL_UP
//    REPEAT
//
void everypio_digital_input_set(every_pin, every_gpio_pull_mode);

// activates the ADC functionality for the pin
void everypio_analog_input_set(every_pin, bool);

// reads the analog values (voltage) set at the
// pin. This reads a 10 bit value from the pin.
int  everypio_analog_read(every_pin);


#endif
