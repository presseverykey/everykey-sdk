#include "../../anykey/anykey.h"

typedef struct any_pin {
  uint8_t port;
  uint8_t pin;
  HW_RW   *iocon;
} any_pin;

#define LED       (any_pin){0,7, &IOCON->PIO0_7}
#define KEY_REV1  (any_pin){1,4, &IOCON->PIO1_4}
#define KEY1_REV2 (any_pin){0,1, &IOCON->PIO0_1}
#define KEY2_REV2 (any_pin){0,0, &IOCON->PIO0_0}
#define PIN_0_1   (any_pin){0,1, &IOCON->PIO0_1}
#define PIN_0_2   (any_pin){0,2, &IOCON->PIO0_2}
#define PIN_0_4   (any_pin){0,4, &IOCON->PIO0_4}
#define PIN_0_5   (any_pin){0,5, &IOCON->PIO0_5}
#define PIN_0_6   (any_pin){0,6, &IOCON->PIO0_6}
#define PIN_0_7   (any_pin){0,7, &IOCON->PIO0_7}
#define PIN_0_8   (any_pin){0,8, &IOCON->PIO0_8}
#define PIN_0_9   (any_pin){0,9, &IOCON->PIO0_9}
#define PIN_0_10  (any_pin){0,10,&IOCON->PIO0_10}
#define PIN_0_11  (any_pin){0,11,&IOCON->PIO0_11}
#define PIN_1_0   (any_pin){1,0, &IOCON->PIO1_0}
#define PIN_1_1   (any_pin){1,1, &IOCON->PIO1_1}
#define PIN_1_2   (any_pin){1,2, &IOCON->PIO1_2}
#define PIN_1_3   (any_pin){1,3, &IOCON->PIO1_3}
#define PIN_1_4   (any_pin){1,4, &IOCON->PIO1_4}
#define PIN_1_5   (any_pin){1,5, &IOCON->PIO1_5}
#define PIN_1_6   (any_pin){1,6, &IOCON->PIO1_6}
#define PIN_1_7   (any_pin){1,7, &IOCON->PIO1_7}
#define PIN_2_0   (any_pin){2,0, &IOCON->PIO2_0}
#define PIN_2_1   (any_pin){2,1, &IOCON->PIO2_1}
#define PIN_2_11  (any_pin){2,11,&IOCON->PIO2_11}
#define PIN_2_2   (any_pin){2,2, &IOCON->PIO2_2}
#define PIN_2_3   (any_pin){2,3, &IOCON->PIO2_3}


// turns the indicated pin on or off.
void anypio_write(any_pin, bool);

// reads the value set at the indicated pin.
// possible values for pull_mode are:
//    NONE
//    PULL_DOWN
//    PULL_UP
//    REPEAT 
bool anypio_read(any_pin, any_gpio_pull_mode);

// activates the ADC functionality for the pin
void anypio_analog_set(any_pin, bool);

// reads the analog values (voltage) set at the
// pin. This reads a 10 bit value from the pin.
int  anypio_analog_read(any_pin);



