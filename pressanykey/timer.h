#ifndef _TIMER_
#define _TIMER_

#include "types.h"

/** identifies one of the hardware timers / counters */
typedef enum {
	CT16B0 = 0,
	CT16B1 = 1,
	CT32B0 = 2,
	CT32B1 = 3
} TimerId;

/** Timer registers - Chapter 15 Tables: 253/4 */
typedef struct {
	HW_RW IR;    // Interrupt register
	HW_RW TCR;   // Timer control register
	HW_RW TC;    // Timer counter (16 bit counters: lower bits)
	HW_RW PR;    // max value for PC
	HW_RW PC;    // Prescale counter
	HW_RW MCR;   // Match Control Register
	HW_RW MR[4];   // Match Registers
	HW_RW CCR;   // Capture Control Register
	HW_RO CR0;   // Capture Register
	HW_RS RESERVED1[3];
	HW_RW EMR;   // External Match Register
	HW_RS RESERVED2[12];
	HW_RW CTCR;  // Count Control Register
	HW_RW PWMC;  // PWM Control Register
	HW_RS PADDING[4066];	//increase struct size to 0x4000
} TIMER_STRUCT;

#define TIMER ((TIMER_STRUCT*)(0x4000C000))

/** Timer interrupt register bits - UM Chap 15.8.1 , Table 255 */
typedef enum {
	TIMER_MR0INT = 1,		// Interrupt flag for channel 0
	TIMER_MR1INT = 1 << 1,  //         "                  1
	TIMER_MR2INT = 1 << 2,  //         "                  2
	TIMER_MR3INT = 1 << 3,  //         "                  3
	TIMER_CR0INT = 1 << 4   // Interrupt flag for capture channel 0 event
} TIMER_INTERRUPT;

/** Timer control register (TCR) bits - UM 15.8.2 Table 256 */
typedef enum {
	TIMER_CEN 	  	= 1,     // 1 = TC and PC are enabled for counting 0=disabled
	TIMER_CRESET	= 1 << 1 //1= TC and PC synchronously reset on next positive edge of PCLK, counters remain reset until TCR/CREST is set to 0
} TIMER_CONTROL;

typedef enum TMR_MCR {
	MR0I = 1 ,       // Interrupt on MR0: an interrupt is generated when MR0 matches the value in the TC. 
	MR0R = 1 <<  1,  // Reset on MR0: the TC will be reset if MR0 matches it. 
	MR0S = 1 <<  2,  // Stop on MR0: the TC and PC will be stopped and TCR[0] will be set to 0 if MR0 matches 0 the TC.
	MR1I = 1 <<  3,
	MR1R = 1 <<  4,
	MR1S = 1 <<  5,
	MR2I = 1 <<  6,
	MR2R = 1 <<  7,
	MR2S = 1 <<  8,
	MR3I = 1 <<  9,
	MR3R = 1 << 10,
	MR3S = 1 << 11,
} TMR_MCR;

typedef enum TMR_CCR {
	CAP0RE = 1,      // Capture on CT16Bn_CAP0 rising edge: a sequence of 0 then 1 on CT16Bn_CAP0 will cause CR0 to be loaded with the contents of TC.
	CAP0FE = 1 << 1, // Capture on CT16Bn_CAP0 falling edge: a sequence of 1 then 0 on CT16Bn_CAP0 will cause CR0 to be loaded with the contents of TC.
	CAP01  = 1 << 2  // Interrupt on CT16Bn_CAP0 event: a CR0 load due to a CT16Bn_CAP0 event will 0 generate an interrupt.
} TMR16_CCR;

// UM Chap 264. §15.8.10
typedef enum TMR_EMR {
	EM0 = 1,
	EM1 = 1 << 1,
	EM2 = 1 << 2,
	EM3 = 1 << 3,
	
	EMC0 = 0x3 << 4,
	EMC1 = 0x3 << 6,
	EMC2 = 0x3 << 8,
	EMC3 = 0x3 << 10
	
	// NOTHING = 0
	// CLEAR   = 1
	// SET     = 2
	// TOGGLE  = 3
} TMR_EMR;


// UM Chap 264. §15.8.10
typedef enum TMR_EMR_CTRL {
	EMC0_NOTHING = 0x00,
	EMC0_CLEAR   = 1 << 4,
	EMC0_SET     = 2 << 4,
	EMC0_TOGGLE  = 3 << 4,

	EMC1_NOTHING = 0x00,
	EMC1_CLEAR   = 1 << 6,
	EMC1_SET     = 2 << 6,
	EMC1_TOGGLE  = 3 << 6,

	EMC2_NOTHING = 0x00,
	EMC2_CLEAR   = 1 << 8,
	EMC2_SET     = 2 << 8,
	EMC2_TOGGLE  = 3 << 8,

	EMC3_NOTHING = 0x00,
	EMC3_CLEAR   = 1 << 10,
	EMC3_SET     = 2 << 10,
	EMC3_TOGGLE  = 3 << 10,
} TMR_EMR_CTRL;

/** Timer count mode (CTCR register values) */
typedef enum { 
	COUNTMODE_TIMER   = 0,		//Timer mode: Count on clock
	COUNTMODE_RISING  = 0x01,	//Count on rising edge
	COUNTMODE_FALLING = 0x02,	//Count on falling edge
	COUNTMODE_BOTH    = 0x03	//Count on both edges
} TIMER_COUNTMODE;

/** Timer PWM control register values */
typedef enum {
	PWMEN0 = 1,         // 0 = CT16Bn_MAT0 is controlled by EM0.
						// 1 = PWM mode is enabled for CT16Bn_MAT0.
	PWMEN1 = 1 << 1,
	PWMEN2 = 1 << 2,
	PWMEN3 = 1 << 3     // Note: It is recommended to use to set the PWM cycle because it is not pinned out.
} TIMER_PWMC_PWMC;


typedef enum {
	TIMER_MATCH_INTERRUPT = 1,
	TIMER_MATCH_RESET = 2,
	TIMER_MATCH_STOP = 4
} TIMER_MATCH_BEHAVIOUR;

/** enables or disables a timer. Note that this does not start it.
 	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1)
 	@param on true to enable, false to disable */
void Timer_Enable(TimerId timer, bool on);

/** queries a timer's counter value.
	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1)
	@return the timer's counter */
uint32_t Timer_GetValue(TimerId timer);

/** sets a timer's prescale counter (clock divider)
 	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1)
 	@param prescale clock divider to set */
void Timer_SetPrescale(TimerId timer, uint32_t prescale);

/** sets a timer's match value
 	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1)
 	@param matchIdx index of match value (0 to 3)
 	@param value match value (up to 65535 for 16 bit timers) */
void Timer_SetMatchValue(TimerId timer, uint8_t matchIdx, uint32_t value);

/** defines what to do when a timer hits a match value.
	If interrupts are triggered, their handler has the signature "void ctXXbY_handler()" (XX=16 or 32bit, Y=Timer 0 or 1)
 	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1)
 	@param matchIdx index of match value (0 to 3)
 	@param behaviour (ORed TIMER_BEHAVIOUR values: Interrupt, Reset and/or Stop - 0 for nothing) */
void Timer_SetMatchBehaviour(TimerId timer, uint8_t matchIdx, uint8_t behaviour);

/** starts a timer
 	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1) */
void Timer_Start(TimerId timer);

/** stops a timer
 	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1) */
void Timer_Stop(TimerId timer);

/** resets a timer. Running state will not be changed.
	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1) */
void Timer_Reset(TimerId timer);

/** Queries the interrupt mask - should be called from within an
interrupt handler to determine the cause of the interrupt
	@param timer timer to query (CT16B0, CT16B1, CT32B0 or CT32B1)
	@return an ORed mask of TIMER_INTERRUPT values */
uint32_t Timer_GetInterruptMask(TimerId timer);

/** Clears fields in the interrupt mask.
Interrupt handlers should clear handled interrupts.
	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1)
	@param mask ORed TIMER_INTERRUPT values to clear */
void Timer_ClearInterruptMask(TimerId timer, uint32_t mask);

/** Enable or disable a Timer's MAT output as PWM.
	Note that additionally, the pin function must be set to MAT out
	and the match behaviour should be set to 0.
	@param timer timer to modify (CT16B0, CT16B1, CT32B0 or CT32B1)
	@param matIdx index of the MAT pin / register (e.g. 2 for MAT2)
	@param enable true to turn PWM on, false to turn it off */

void Timer_EnablePWM(TimerId timer, uint8_t matIdx, bool enable);



#endif
