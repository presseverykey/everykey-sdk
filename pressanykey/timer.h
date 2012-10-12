#ifndef _TIMER_
#define _TIMER_

#include "types.h"

// Chapter 15 Tables: 253/4
typedef struct {
	HW_RW IR;    // Interrupt register
	HW_RW TCR;   // Timer control register
	HW_RW TC;    // lower 16 bits = Timer counter
	HW_RW PR;    // max value for PC
	HW_RW PC;    // Prescale counter
	HW_RW MCR;   // Match Control Register
	HW_RW MR0;   // Match Registers
	HW_RW MR1;
	HW_RW MR2;
	HW_RW MR3;
	HW_RW CCR;   // Capture Control Register
	HW_RO CR0;   // Capture Register
	HW_RW EMR;   // External Match Register
	HW_RS RESERVED[0xC];
	HW_RW CTCR;  // Count Control Register
	HW_RW PWMC;  // PWM Control Register
} TMR_STRUCT;

#define TMR16B0 ((TMR_STRUCT*)(0x4000C000))
#define TMR16B1 ((TMR_STRUCT*)(0x40010000))

#define TMR32B0 ((TMR_STRUCT*)(0x40014000))
#define TMR32B1 ((TMR_STRUCT*)(0x40018000))

// UM Chap 15.8.1 , Table 255, bits for IR
typedef enum TMR_IR {
	MR0INT = 1,				// Interrupt flag for channel 0
	MR1INT = 1 << 1,  //         "                  1
	MR2INT = 1 << 2,  //         "                  2
	MR3INT = 1 << 3,  //         "                  3
	CR0INT = 1 << 4   // Interrupt flag for capture channel 0 event
} TMR_IR;

// UM 15.8.2 Table 256
typedef enum TMR_TCR {
	CEN   = 1,     // 1 = TC and PC are enabled for counting 0=disabled
	CREST = 1 << 1 //1= TC and PC synchronously reset on next positive edge of PCLK,
	               // counters remain reset until TCR/CREST is set to 0
} TMR_TCR;

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

typedef enum TMR_CTCR {
	CTM = 0x3, // Counter/Timer Mode. This field selects which rising PCLK edges 
						 // can increment Timer’s Prescale Counter (PC), or clear PC and 
						 // increment Timer Counter (TC).
	CIS = 0x3 << 2 // Counter Input Select (de facto Reserved)
} TMR_CCR;

typedef enum TMR_CTCR_CTM {
	TIMER       = 0,
	CTR_RISING  = 0x01,
	CTR_FALLING = 0x02,
	CTR_BOTH    = 0x03
} TMR_CCR_CTM;

typedef enum TMR_PWMC {
	PWMEN0 = 1,         // 0 = CT16Bn_MAT0 is controlled by EM0.
											// 1 = PWM mode is enabled for CT16Bn_MAT0.
	PWMEN1 = 1 << 1,
	PWMEN2 = 1 << 2,
	PWMEN3 = 1 << 3     // Note: It is recommended to use to set the PWM cycle because it is not pinned out.
} TMR_PWMC;

#endif
