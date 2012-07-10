#include "types.h"
#include "memorymap.h"


//typedef struct {
//  unsigned FUNC:3;
//  unsigned MODE:2;
//  unsigned HYS:1;
//  unsigned RESERVED0:1;
//  unsigned ADMODE:1;
//  unsigned RESERVED1:2;
//  unsigned OD:1;
//  unsigned RESERVED2:21;
//} IOCON_ADC;

/*
  Description UM10375 Chapter 20:

  The ADC is configured using the following registers:

	1. Pins: The ADC pin functions are configured in the IOCONFIG register
	block (Section 7.4).

	2. Power and peripheral clock: In the SYSAHBCLKCTRL register, set bit
	13 (Table 25). Power to the ADC at run-time is controlled through the
	PDRUNCFG register (Table 55).
*/

void ADC_Init();

/* A/D Control Register 20.6.1*/
typedef struct {
	unsigned SEL0     :1;
	unsigned SEL1     :1;
	unsigned SEL2     :1;
	unsigned SEL3     :1;
	unsigned SEL4     :1;
	unsigned SEL5     :1;
	unsigned SEL6     :1;
	unsigned SEL7     :1;
	unsigned CLKDIV   :8;
	unsigned BURST    :1;
  /*
0x0 11 clocks / 10 bits
0x1 10 clocks / 9 bits
0x2 9 clocks / 8 bits
0x3 8 clocks / 7 bits
0x4 7 clocks / 6 bits
0x5 6 clocks / 5 bits
0x6 5 clocks / 4 bits
0x7 4 clocks / 3 bits
  */
	unsigned CLKS     :3;
	unsigned RESERVED1:4;
  /*
0x0 No start (this value should be used when clearing PDN to 0).
0x1 Start conversion now.
0x2 Start conversion when the edge selected by bit 27 occurs on PIO0_2/SSEL/CT16B0_CAP0.
0x3 Start conversion when the edge selected by bit 27 occurs on PIO1_5/DIR/CT32B0_CAP0.
  */
	unsigned START    :3;
	unsigned EDGE     :1;
	unsigned RESERVED0:4;
} ADCTRL;

/* A/D Global Data Register 20.6.2 */
typedef struct {
	unsigned RESERVED0:5;
	unsigned V_VREF:10;
	unsigned RESERVED1:8;
	unsigned CHN:3;
	unsigned RESERVED3:3;
	unsigned OVERRUN:1;
	unsigned DONE:1;
} ADGDR;

/* A/D Interrupt Enable Registeri 20.6.3 */
typedef struct {
	unsigned ADINTEN:8;
	unsigned ADGINTEN:1;
	unsigned RESERVED0:23;
} ADINTEN;

/* A/D Data Registers 20.6.4 */
typedef struct {
	unsigned RESERVED0:6;
	unsigned V_VREF:10;
	unsigned RESERVED1:14;
	unsigned OVERRUN:1;
	unsigned DONE:1;
} ADDR;

/*A/D Status Register 20.6.5 */
typedef struct {
	unsigned DONE:8;
	unsigned OVERRUN:8;
	unsigned ADINT:1;
	unsigned RESERVED0:15;
} ADSTAT;

typedef struct {

	/* A/D Control Register. The AD0CR register must be written to select
	 * the operating mode before A/D conversion can occur.*/
	ADCTRL AD0CR; 

	/* A/D Global Data Register. Contains the result of the most recent
	 * A/D conversion. */
	ADGDR AD0GDR; 

	/*reserved*/
	HW_RS RESERVED;
	
	/* A/D Interrupt Enable Register. This register contains enable bits
	 * that allow the DONE flag of each A/D channel to be included or excluded
	 * from contributing to the generation of an A/D interrupt.*/
	ADINTEN AD0INTEN;

	/* A/D Channel 0 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 0*/
	ADDR AD0DR0;

	/* A/D Channel 1 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 1.*/
	ADDR AD0DR1;

	/* A/D Channel 2 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 2.*/
	ADDR AD0DR2;

	/* A/D Channel 3 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 3.*/
	ADDR AD0DR3;

	/* A/D Channel 4 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 4.*/
	ADDR AD0DR4;

	/* A/D Channel 5 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 5.*/
	ADDR AD0DR5;

	/* A/D Channel 6 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 6.*/

	ADDR AD0DR6;
	/* A/D Channel 7 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 7.*/
	ADDR AD0DR7;

	/* A/D Status Register. This register contains DONE and OVERRUN flags
	 * for all of the A/D channels, as well as the A/D interrupt flag.*/
	ADSTAT AD0STAT;
} ADC_STRUCT;

#define ADC ((ADC_STRUCT)(0x4001C000))
