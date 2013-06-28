#ifndef _GPIO_
#define _GPIO_

#include "types.h"
#include "memorymap.h"



typedef struct any_pin {
  uint8_t port;
  uint8_t pin;
  HW_RW   *iocon;
} any_pin;

typedef enum {
	INPUT = 0,
	OUTPUT
} any_gpio_direction;

typedef enum {
	NONE = 0x00,
	PULL_DOWN = 0x08,
	PULL_UP = 0x10,
	REPEAT = 0x18
} any_gpio_pull_mode;

typedef enum {
	HYSTERESIS_OFF = 0x00,
	HYSTERESIS_ON = 0x20
} any_gpio_hysteresis_mode;

typedef enum {
	ADMODE_ANALOG  = 0x00,
	ADMODE_DIGITAL = 0x80
} any_gpio_ad_mode;


/** GPIO input events that can cause interrupts */
typedef enum {
	TRIGGER_NONE = 0,		     //No interrupt trigger
	TRIGGER_RISING_EDGE = 1, //Rising edge triggers interrupt
	TRIGGER_FALLING_EDGE = 2,//Falling edge triggers interrupt
	TRIGGER_BOTH_EDGES = 3,	 //Both edges trigger interrupts
	TRIGGER_HIGH_LEVEL = 4,	 //High level causes interrupt
	TRIGGER_LOW_LEVEL = 5	   //Low level causes interrupt
} any_gpio_interrupt_mode;

#define LED       (any_pin){0,7, &IOCON->PIO0_7}
#define KEY_REV1  (any_pin){1,4, &IOCON->PIO1_4}
#define KEY1_REV2 (any_pin){0,1, &IOCON->PIO0_1}
#define KEY2_REV2 (any_pin){0,0, &IOCON->PIO0_0}
#define PIN_0_8   (any_pin){0,8, &IOCON->PIO0_8}
#define PIN_0_9   (any_pin){0,9, &IOCON->PIO0_9}
#define PIN_2_11  (any_pin){2,11,&IOCON->PIO2_11}
#define PIN_1_5   (any_pin){1,5, &IOCON->PIO1_5}
#define PIN_1_3   (any_pin){1,3, &IOCON->PIO1_3}
#define PIN_2_3   (any_pin){2,3, &IOCON->PIO2_3}
#define PIN_1_4   (any_pin){1,4, &IOCON->PIO1_4}
#define PIN_1_2   (any_pin){1,2, &IOCON->PIO1_2}
#define PIN_1_1   (any_pin){1,1, &IOCON->PIO1_1}
#define PIN_2_2   (any_pin){2,2, &IOCON->PIO2_2}
#define PIN_1_7   (any_pin){1,7, &IOCON->PIO1_7}
#define PIN_1_6   (any_pin){1,6, &IOCON->PIO1_6}
#define PIN_0_1   (any_pin){0,1, &IOCON->PIO0_1}
#define PIN_2_0   (any_pin){2,0, &IOCON->PIO2_0}
#define PIN_0_4   (any_pin){0,4, &IOCON->PIO0_4}
#define PIN_1_0   (any_pin){1,0, &IOCON->PIO1_0}
#define PIN_0_6   (any_pin){0,6, &IOCON->PIO0_6}
#define PIN_0_11  (any_pin){0,11,&IOCON->PIO0_11}
#define PIN_0_10  (any_pin){0,10,&IOCON->PIO0_10}
#define PIN_0_2   (any_pin){0,2, &IOCON->PIO0_2}
#define PIN_0_7   (any_pin){0,7, &IOCON->PIO0_7}
#define PIN_0_5   (any_pin){0,5, &IOCON->PIO0_5}
#define PIN_2_1   (any_pin){2,1, &IOCON->PIO2_1}



/** sets the direction of a GPIO pin.
	@param port the port 
	@param port the pin
	@param direction the direction
*/ 
void any_gpio_set_dir(uint8_t port, uint8_t pin, any_gpio_direction dir);

/** writes a bit to a GPIO output pin
	@param port the port 
	@param port the pin
	@param value the value
*/ 
void any_gpio_write(uint8_t port, uint8_t pin, bool value);

/** reads the state of a GPIO output pin
	@param port the port 
	@param port the pin
	@return the value
*/ 
bool any_gpio_read(uint8_t port, uint8_t pin);

/** sets the pullup/pulldown resistors of an IO pin. 
	@param pin pointer to a pin register in the IOCON struct. Must be a pin that supports pullup/pulldown, in a supported mode
	@param mode mode to set to
*/
void any_gpio_set_pull(HW_RW* pin, any_gpio_pull_mode mode);

/** define to call setPull in a syntax similar to other in/out calls if
 * port and pin are known at compile time. Two-step for argument macro
 * expansion. */

#define _SETPULL2(port,pin,mode) {any_gpio_set_pull(&(IOCON->PIO ## port ## _ ## pin),mode); }
#define ANY_GPIO_SETPULL(port,pin,mode) _SETPULL2(port,pin,mode)


/** sets the hysteresis mode of an IO pin. 
  @param pin pointer to a pin register in the IOCON struct. Must be a
  pin that supports hysteresis, in a supported mode @param mode mode to
  set to
*/
void any_gpio_set_hysteresis(HW_RW* pin, any_gpio_hysteresis_mode mode);

/** define to call setHysteresis in a syntax similar to other in/out
 * calls if port and pin are known at compile time. Two-step for
 * argument macro expansion. */
#define _SETHYSTERESIS2(port,pin,mode) {any_gpio_set_hysteresis(&(IOCON->PIO ## port ## _ ## pin),mode); }
#define ANY_GPIO_SETHYSTERESIS(port,pin,mode) _SETHYSTERESIS2(port,pin,mode)

/** sets the pin function of an IO pin
  sets the digital pin functions of an IO pin. precise functions support by the pins 
  vary, please consult memorymap.h.

  To set up a pin for analog reading, see `any_gpio_set_analog()` below.

	@param pin pointer to a pin register in the IOCON struct. 
	@param function pin function to use (e.g. ADC or TIMER). Obviously, the given pin must support that function
	@param admode analog or digital pin mode (typically analog for ADC, digital for all others) 
*/
void any_gpio_set_function(HW_RW* pin, IOCON_IO_FUNC mode, IOCON_IO_ADMODE admode); 

/** define to call setFunction in a syntax similar to other in/out calls
 * if port and pin are known at compile time. Two-step for argument
 * macro expansion. 

e.g. GPIO_SETFUNCTION(0, 10, TMR, IOCON_IO_ADMODE_DIGITAL); */

#define GPIO_SETFUNCTION(port,pin,func,admode) _SETFUNCTION2(port,pin,func,admode)
#define _SETFUNCTION2(port,pin,func,admode) {any_gpio_set_function(&(IOCON->PIO ## port ## _ ## pin),IOCON_IO_FUNC_PIO ## port ## _ ## pin ## _ ## func ,admode); }



/** sets the interrupt behaviour of a given port and pin.
	The handler has the signature "void gpioX_handler()" (X = port, 0-3)
	@param port the GPIO port
	@param pin the GPIO pin
	@param mode the interrupt behaviour */

void any_gpio_set_interrupt_mode(uint8_t port, uint8_t pin, any_gpio_interrupt_mode mode);

/** returns the pins that caused an interrupt.
	Interrupt handlers should call this to determine the cause of the interrupt.
	@param port the GPIO port to query
	@return the currently active or pending interrupt mask, ORed pins */
uint32_t any_gpio_get_interrupt_mask(uint8_t port);

/** clears pins from the interrupt mask.
 	This function should be called during the corresponding interrupt handler to clear handled interrupts.
	@param port the GPIO port to modify
	@param mask ORed pins to clear */
void any_gpio_clear_interrupt_mask(uint8_t port, uint32_t mask);


#endif
