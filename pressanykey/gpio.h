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
	@param mode mode to set to (e.g. ADC or TIMER). Obviously, the given pin must support that function */
void GPIO_SetFunction(HW_RW* pin, IOCON_IO_FUNC function);

/** define to call setFunction in a syntax similar to other in/out calls if port and pin are known at compile time. Two-step for argument macro expansion. */
#define GPIO_SETFUNCTION(port,pin,func) GPIO_SETFUNCTION2(port,pin,func)
#define GPIO_SETFUNCTION2(port,pin,func) {GPIO_SetFunction(&(IOCON->PIO ## port ## _ ## pin),IOCON_IO ## port ## _ ## pin ## _FUNC_ ## func); }


#endif
