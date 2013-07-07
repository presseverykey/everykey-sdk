#ifndef ANYPIO_H
#define ANYPIO_H

#include "anykey/anykey.h"


// These values could (and typically should) be looked up via the IOCON
// struct in `syscon.h`, but we need them as const values for the intializers
// below.

#define IOCON_BASE         0x40044000
#define IOCON_PIO2_6       (IOCON_BASE + 0x00)
#define IOCON_PIO2_0       (IOCON_BASE + 0x008)
#define IOCON_RESET_PIO0_0 (IOCON_BASE + 0x00C)
#define IOCON_PIO0_1       (IOCON_BASE + 0x010)
#define IOCON_PIO1_8       (IOCON_BASE + 0x014)
#define IOCON_PIO0_2       (IOCON_BASE + 0x01C)
#define IOCON_PIO2_7       (IOCON_BASE + 0x020)
#define IOCON_PIO2_8       (IOCON_BASE + 0x024)
#define IOCON_PIO2_1       (IOCON_BASE + 0x028)
#define IOCON_PIO0_3       (IOCON_BASE + 0x02C)
#define IOCON_PIO0_4       (IOCON_BASE + 0x030)
#define IOCON_PIO0_5       (IOCON_BASE + 0x034)
#define IOCON_PIO1_9       (IOCON_BASE + 0x038)
#define IOCON_PIO3_4       (IOCON_BASE + 0x03C)
#define IOCON_PIO2_4       (IOCON_BASE + 0x040)
#define IOCON_PIO2_5       (IOCON_BASE + 0x044)
#define IOCON_PIO3_5       (IOCON_BASE + 0x048)
#define IOCON_PIO0_6       (IOCON_BASE + 0x04C)
#define IOCON_PIO0_7       (IOCON_BASE + 0x050)
#define IOCON_PIO2_9       (IOCON_BASE + 0x054)
#define IOCON_PIO2_10      (IOCON_BASE + 0x058)
#define IOCON_PIO2_2       (IOCON_BASE + 0x05C)
#define IOCON_PIO0_8       (IOCON_BASE + 0x060)
#define IOCON_PIO0_9       (IOCON_BASE + 0x064)
#define IOCON_SWCLK_PIO0_10 (IOCON_BASE + 0x068)
#define IOCON_PIO1_10      (IOCON_BASE + 0x06C)
#define IOCON_PIO2_11      (IOCON_BASE + 0x070)
#define IOCON_R_PIO0_11    (IOCON_BASE + 0x074)
#define IOCON_R_PIO1_0     (IOCON_BASE + 0x078)
#define IOCON_R_PIO1_1     (IOCON_BASE + 0x07C)
#define IOCON_R_PIO1_2     (IOCON_BASE + 0x080)
#define IOCON_PIO3_0       (IOCON_BASE + 0x080)
#define IOCON_PIO3_1       (IOCON_BASE + 0x084)
#define IOCON_PIO2_3       (IOCON_BASE + 0x08C)
#define IOCON_SWDIO_PIO1_3 (IOCON_BASE + 0x090)
#define IOCON_PIO1_4       (IOCON_BASE + 0x094)
#define IOCON_PIO1_11      (IOCON_BASE + 0x098)
#define IOCON_PIO3_2       (IOCON_BASE + 0x09C)
#define IOCON_PIO1_5       (IOCON_BASE + 0x0A0)
#define IOCON_PIO1_6       (IOCON_BASE + 0x0A4)
#define IOCON_PIO1_7       (IOCON_BASE + 0x0A8)
#define IOCON_PIO3_3       (IOCON_BASE + 0x0AC)
#define IOCON_SCK0_LOC     (IOCON_BASE + 0x0B0)
#define IOCON_DSR_LOC      (IOCON_BASE + 0x0B4)
#define IOCON_DCD_LOC      (IOCON_BASE + 0x0B8)
#define IOCON_RI_LOC       (IOCON_BASE + 0x0BC)


typedef struct any_pin {
  uint8_t port;
  uint8_t pin;
  HW_RW   *iocon;
} any_pin;

#define LED       (any_pin){0,7, IOCON_PIO0_7}
#define KEY_REV1  (any_pin){1,4, IOCON_PIO1_4}
#define KEY1_REV2 (any_pin){0,1, IOCON_PIO0_1}
#define KEY2_REV2 (any_pin){0,0, IOCON_RESET_PIO0_0}
#define PIN_0_1   (any_pin){0,1, IOCON_PIO0_1}
#define PIN_0_2   (any_pin){0,2, IOCON_PIO0_2}
#define PIN_0_4   (any_pin){0,4, IOCON_PIO0_4}
#define PIN_0_5   (any_pin){0,5, IOCON_PIO0_5}
#define PIN_0_6   (any_pin){0,6, IOCON_PIO0_6}
#define PIN_0_7   (any_pin){0,7, IOCON_PIO0_7}
#define PIN_0_8   (any_pin){0,8, IOCON_PIO0_8}
#define PIN_0_9   (any_pin){0,9, IOCON_PIO0_9}
#define PIN_0_10  (any_pin){0,10,IOCON_SWCLK_PIO0_10}
#define PIN_0_11  (any_pin){0,11,IOCON_R_PIO0_11}
#define PIN_1_0   (any_pin){1,0, IOCON_R_PIO1_0}
#define PIN_1_1   (any_pin){1,1, IOCON_R_PIO1_1}
#define PIN_1_2   (any_pin){1,2, IOCON_R_PIO1_2}
#define PIN_1_3   (any_pin){1,3, IOCON_SWDIO_PIO1_3}
#define PIN_1_4   (any_pin){1,4, IOCON_PIO1_4}
#define PIN_1_5   (any_pin){1,5, IOCON_PIO1_5}
#define PIN_1_6   (any_pin){1,6, IOCON_PIO1_6}
#define PIN_1_7   (any_pin){1,7, IOCON_PIO1_7}
#define PIN_2_0   (any_pin){2,0, IOCON_PIO2_0}
#define PIN_2_1   (any_pin){2,1, IOCON_PIO2_1}
#define PIN_2_11  (any_pin){2,11,IOCON_PIO2_11}
#define PIN_2_2   (any_pin){2,2, IOCON_PIO2_2}
#define PIN_2_3   (any_pin){2,3, IOCON_PIO2_3}


// turns the indicated pin on or off.
void anypio_write(any_pin, bool);

// reads the value set at the indicated pin.
// Before calling this function, pins need to be
// configured as INPUT by calling `anypio_digital_input_set`
// see below.
bool anypio_read(any_pin);


// configures the given pin as a digital input pin,
// the pin needs to be configured for input before
// using `anypio_gpio_read`.
// Along with the pin to configure, it's necessary to
// provide a value of the pull_up/down mode. 
//
// Possible values are:
//    NONE
//    PULL_DOWN
//    PULL_UP
//    REPEAT
//
void anypio_digital_input_set(any_pin, any_gpio_pull_mode);

// activates the ADC functionality for the pin
void anypio_analog_input_set(any_pin, bool);

// reads the analog values (voltage) set at the
// pin. This reads a 10 bit value from the pin.
int  anypio_analog_read(any_pin);


#endif
