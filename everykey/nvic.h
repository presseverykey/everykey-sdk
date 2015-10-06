/***************************************
 Nested vectored interrupt controller
***************************************/

#ifndef _NVIC_
#define _NVIC_

#include "types.h"
#include "memorymap.h"

/** enables a given interrupt. Note this only applies to the NVIC, enabling a specific interrupt might require additional setup in other peripherals.
	@param interrupt index of interrupt to enable
*/
void NVIC_EnableInterrupt(NVIC_INTERRUPT_INDEX interrupt);

/** disables a given interrupt.
	@param interrupt index of interrupt to disable
*/
void NVIC_DisableInterrupt(NVIC_INTERRUPT_INDEX interrupt);

/** returns whether a given interrupt is currently enabled 
	@param interrupt index of interrupt to query
	@return true if the interrupt is enabled, false otherwise
*/
bool NVIC_IsInterruptEnabled(NVIC_INTERRUPT_INDEX interrupt);

/** marks a given interrupt as pending
	@param interrupt index of interrupt to set pending
*/
void NVIC_SetInterruptPending(NVIC_INTERRUPT_INDEX interrupt);

/** unmarks a given interrupt as pending
	@param interrupt index of interrupt to clear pending
*/
void NVIC_ClearInterruptPending(NVIC_INTERRUPT_INDEX interrupt);

/** returns whether a given interrupt is pending
	@param interrupt index of interrupt to query
	@return true if the interrupt is pending, false otherwise
*/
bool NVIC_IsInterruptPending(NVIC_INTERRUPT_INDEX interrupt);

/** returns whether a given interrupt is currently active 
	@param interrupt index of interrupt to query
	@return true if the interrupt is active, false otherwise
*/
bool NVIC_IsInterruptActive(NVIC_INTERRUPT_INDEX interrupt);

/** sets the priority of a given interrupt. Note that the values aren't taken exactly - the LPC has 8 levels, resulting in steps of 32.
	@param interrupt index of interrupt to modify
	@param prio the new interrupt priority (0..255, 0 highest)
*/
void NVIC_SetInterruptPriority(NVIC_INTERRUPT_INDEX interrupt, uint8_t prio);

/** returns the current priority of a given interrupt. See NVIC_SetInterruptPriority remarks.
	@param interrupt index of interrupt to query
	@return the interrupt priority (0..255, 0 highest)
*/
uint8_t NVIC_GetInterruptPriority(NVIC_INTERRUPT_INDEX interrupt);

/** sets the priority of a given system handler. Note that the values aren't taken exactly - the LPC has 8 levels, resulting in steps of 32.
	@param syshandler index of system handler to modify
	@param prio the new system handler priority (0..255, 0 highest)
*/
void NVIC_SetSystemHandlerPriority(SCB_SYSTEM_HANDLER_INDEX syshandler, uint8_t prio);

/** returns the current priority of a given system handler. See NVIC_SetSystemHandlerPriority remarks.
	@param syshandler index of system handler to query
	@return the system handler priority (0..255, 0 highest)
*/
uint8_t NVIC_GetSystemHandlerPriority(SCB_SYSTEM_HANDLER_INDEX syshandler);

/** triggers a specified interrupt
	@param interrupt index of interrupt to modify
*/
void NVIC_TriggerInterrupt(NVIC_INTERRUPT_INDEX interrupt);


/* The following functions are not really NVIC, but SCB. Anyway, since it's related to interrupts,
 we put it here so it's easier to find. */

/** returns the number of bits within interrupt priorities used for interrupt group priority. Interrupt priorities are separated into group priority and subpriority (priority within the group). Interrupts priority is still chosen among the whole priority field, but preemption only happens among priority groups. In other words: Interrupts with the same priority groups do not interrupt each other. Note that not all processors support all priority bits. In this case, the lowest bits are ignored.
	@return number of bits used for interrupt group priority (0..7) - only 0-3 make sense. */
uint8_t NVIC_GetInterruptGroupPriorityBits();

/** sets the number of bits within interrupt priorities used for interrupt group priority. See NVIC_GetInterruptGroupPriorityBits.
 @param groupLength number of bits to be used for interrupt group priority (0..7) - only 0-3 make sense. */
void NVIC_SetInterruptGroupPriorityBits(uint8_t groupLength);

/** requests a system reset */
void NVIC_ResetSystem();
	


#endif
