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
	unsigned SEL      :8;
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

typedef enum ADC_BURST {
  ADC_BURST_SW = 0, 
  ADC_BURST_HW = 1, 
} ADC_BURST;


typedef enum ADC_CLKS {
  ADC_CLKS_11C_10B = 0x0,
  ADC_CLKS_10C_9B  = 0x1,
  ADC_CLKS_9C_8B    =0x2,
  ADC_CLKS_8C_7B    =0x3,
  ADC_CLKS_7C_6B    =0x4,
  ADC_CLKS_6C_5B    =0x5,
  ADC_CLKS_5C_4B    =0x6,
  ADC_CLKS_4C_3B    =0x7,
} ADC_CLKS;

typedef enum ADC_START {
  ADC_START_NOSTART = 0x0,
  ADC_START_START   = 0x1,
  ADC_START_16C0    = 0x2,
  ADC_START_32C0    = 0x3,
  ADC_START_32M0    = 0x4,
  ADC_START_32M1    = 0x5,
  ADC_START_16M0    = 0x6,
  ADC_START_16M1    = 0x7,
} ADC_START;

typedef enum ADC_EDGE {
  ADC_EDGE_RISING  = 0x0,
  ADC_EDGE_FALLING = 0x1,
} ADC_EDGE;

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
	volatile ADCTRL AD0CR; 

	/* A/D Global Data Register. Contains the result of the most recent
	 * A/D conversion. */
	volatile ADGDR AD0GDR; 

	/*reserved*/
	HW_RS RESERVED;
	
	/* A/D Interrupt Enable Register. This register contains enable bits
	 * that allow the DONE flag of each A/D channel to be included or excluded
	 * from contributing to the generation of an A/D interrupt.*/
	volatile ADINTEN AD0INTEN;

	/* A/D Channel 0-7 Data Register. This register contains the result of
	 * the most recent conversion completed on channel 0*/
	volatile ADDR AD0DR [8];

	/* A/D Status Register. This register contains DONE and OVERRUN flags
	 * for all of the A/D channels, as well as the A/D interrupt flag.*/
	volatile ADSTAT AD0STAT;
} ADC_STRUCT;

#define ADC ((ADC_STRUCT*)(0x4001C000))
