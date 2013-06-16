#ifndef _GPIO_
#define _GPIO_

#include "types.h"
#include "memorymap.h"

typedef enum GPIO_Direction {
	GPIO_Input = 0,
	GPIO_Output
} GPIO_Direction;

/** sets the direction of a GPIO pin.
	@param port the port 
	@param port the pin
	@param direction the direction
*/ 
void GPIO_SetDir(uint8_t port, uint8_t pin, GPIO_Direction dir);

/** writes a bit to a GPIO output pin
	@param port the port 
	@param port the pin
	@param value the value
*/ 
void GPIO_WriteOutput(uint8_t port, uint8_t pin, bool value);

/** reads the state of a GPIO output pin
	@param port the port 
	@param port the pin
	@return the value
*/ 
bool GPIO_ReadInput(uint8_t port, uint8_t pin);

/** sets the pullup/pulldown resistors of an IO pin. 
	@param pin pointer to a pin register in the IOCON struct. Must be a pin that supports pullup/pulldown, in a supported mode
	@param mode mode to set to
*/
void GPIO_SetPull(HW_RW* pin, IOCON_IO_PULL_MODE mode);

/** define to call setPull in a syntax similar to other in/out calls if port and pin are known at compile time. Two-step for argument macro expansion. */
#define GPIO_SETPULL2(port,pin,mode) {GPIO_SetPull(&(IOCON->PIO ## port ## _ ## pin),mode); }
#define GPIO_SETPULL(port,pin,mode) GPIO_SETPULL2(port,pin,mode)

/** sets the hysteresis mode of an IO pin. 
	@param pin pointer to a pin register in the IOCON struct. Must be a pin that supports hysteresis, in a supported mode
	@param mode mode to set to
*/
void GPIO_SetHysteresis(HW_RW* pin, IOCON_IO_HYSTERESIS_MODE mode);

/** define to call setHysteresis in a syntax similar to other in/out calls if port and pin are known at compile time. Two-step for argument macro expansion. */
#define GPIO_SETHYSTERESIS2(port,pin,mode) {GPIO_SetHysteresis(&(IOCON->PIO ## port ## _ ## pin),mode); }
#define GPIO_SETHYSTERESIS(port,pin,mode) GPIO_SETHYSTERESIS2(port,pin,mode)

/** sets the pin function of an IO pin
	@param pin pointer to a pin register in the IOCON struct. 
	@param function pin function to use (e.g. ADC or TIMER). Obviously, the given pin must support that function
	@param admode analog or digital pin mode (typically analog for ADC, digital for all others) 
*/
void GPIO_SetFunction(HW_RW* pin, IOCON_IO_FUNC function, IOCON_IO_ADMODE admode);

/** define to call setFunction in a syntax similar to other in/out calls if port and pin are known at compile time. Two-step for argument macro expansion. 
e.g. GPIO_SETFUNCTION(0, 10, TMR, IOCON_IO_ADMODE_DIGITAL); */
#define GPIO_SETFUNCTION(port,pin,func,admode) GPIO_SETFUNCTION2(port,pin,func,admode)
#define GPIO_SETFUNCTION2(port,pin,func,admode) {GPIO_SetFunction(&(IOCON->PIO ## port ## _ ## pin),IOCON_IO_FUNC_PIO ## port ## _ ## pin ## _ ## func ,admode); }

/** GPIO input events that can cause interrupts */
typedef enum {
	GPIO_INTERRUPT_NONE = 0,		//No interrupt trigger
	GPIO_INTERRUPT_RISING_EDGE = 1,	//Rising edge triggers interrupt
	GPIO_INTERRUPT_FALLING_EDGE = 2,//Falling edge triggers interrupt
	GPIO_INTERRUPT_BOTH_EDGES = 3,	//Both edges trigger interrupts
	GPIO_INTERRUPT_HIGH_LEVEL = 4,	//High level causes interrupt
	GPIO_INTERRUPT_LOW_LEVEL = 5	//Low level causes interrupt
} GPIO_INTERRUPT_TRIGGER;

/** sets the interrupt behaviour of a given port and pin.
	The handler has the signature "void gpioX_handler()" (X = port, 0-3)
	@param port the GPIO port
	@param pin the GPIO pin
	@param mode the interrupt behaviour */
void GPIO_SetInterruptMode(uint8_t port, uint8_t pin, GPIO_INTERRUPT_TRIGGER mode);

/** returns the pins that caused an interrupt.
	Interrupt handlers should call this to determine the cause of the interrupt.
	@param port the GPIO port to query
	@return the currently active or pending interrupt mask, ORed pins */
uint32_t GPIO_GetInterruptMask(uint8_t port);

/** clears pins from the interrupt mask.
 	This function should be called during the corresponding interrupt handler to clear handled interrupts.
	@param port the GPIO port to modify
	@param mask ORed pins to clear */
void GPIO_ClearInterruptMask(uint8_t port, uint32_t mask);


#endif
