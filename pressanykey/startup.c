#include "pressanykey.h"

void bootstrap(void);	//bootstrap that will call main later
void deadend(void);	//neverending handler

/* we define some standard handler names here - they all default to deadend but may be changed by implementing a real function with that name. So if they are triggered but undefined, we'll just stop. DEFAULT_IMP defines a weak alias. */

#define DEFAULTS_TO(func) __attribute__ ((weak, alias (#func)))

void main(void);// DEFAULTS_TO(deadend);
void systick(void) DEFAULTS_TO(deadend);
void usb_fiq_handler(void) DEFAULTS_TO(deadend);
void usb_irq_handler(void) DEFAULTS_TO(deadend);

// Define the vector table - make sure it's mapped to 0x0.
__attribute__ ((section("vectors")))

const VECTOR_TABLE vtable = {
	(void*)0x10001ff0,	//Stack top
	bootstrap,		//boot code
	deadend,		//NMI
	deadend,		//Hard fault
	deadend,		//Memory protection unit fault handler
	deadend,		//Bus fault handler
	deadend,		//Usage fault handler
	deadend,		//RESERVED1
	deadend,		//RESERVED2
	deadend,		//Reserved for CRC checksum
	deadend,		//RESERVED4
	deadend,		//SVCall handler
	deadend,		//Debug monitor handler
	deadend,		//RESERVED5
	deadend,		//PendSV handler
	systick,		//The SysTick handler
	deadend,		//PIO0_0  Wakeup
	deadend,		//PIO0_1  Wakeup
	deadend,		//PIO0_2  Wakeup
	deadend,		//PIO0_3  Wakeup
	deadend,		//PIO0_4  Wakeup
	deadend,		//PIO0_5  Wakeup
	deadend,		//PIO0_6  Wakeup
	deadend,		//PIO0_7  Wakeup
	deadend,		//PIO0_8  Wakeup
	deadend,		//PIO0_9  Wakeup
	deadend,		//PIO0_10  Wakeup
	deadend,		//PIO0_11  Wakeup
	deadend,		//PIO1_0  Wakeup
	deadend,		//PIO1_1  Wakeup
	deadend,		//PIO1_2  Wakeup
	deadend,		//PIO1_3  Wakeup
	deadend,		//PIO1_4  Wakeup
	deadend,		//PIO1_5  Wakeup
	deadend,		//PIO1_6  Wakeup
	deadend,		//PIO1_7  Wakeup
	deadend,		//PIO1_8  Wakeup
	deadend,		//PIO1_9  Wakeup
	deadend,		//PIO1_10  Wakeup
	deadend,		//PIO1_11  Wakeup
	deadend,		//PIO2_0  Wakeup
	deadend,		//PIO2_1  Wakeup
	deadend,		//PIO2_2  Wakeup
	deadend,		//PIO2_3  Wakeup
	deadend,		//PIO2_4  Wakeup
	deadend,		//PIO2_5  Wakeup
	deadend,		//PIO2_6  Wakeup
	deadend,		//PIO2_7  Wakeup
	deadend,		//PIO2_8  Wakeup
	deadend,		//PIO2_9  Wakeup
	deadend,		//PIO2_10  Wakeup
	deadend,		//PIO2_11  Wakeup
	deadend,		//PIO3_0  Wakeup
	deadend,		//PIO3_1  Wakeup
	deadend,		//PIO3_2  Wakeup
	deadend,		//PIO3_3  Wakeup
	deadend,		//I2C
	deadend,		//16-bit Timer 0 handler
	deadend,		//16-bit Timer 1 handler
	deadend,		//32-bit Timer 0 handler
	deadend,		//32-bit Timer 1 handler
	deadend,		//SSP
	deadend,		//UART
	usb_irq_handler,	//USB IRQ
	usb_fiq_handler,	//USB FIQ
	deadend,		//ADC
	deadend,		//WDT
	deadend,		//BOD
	deadend,		//Flash
	deadend,		//PIO INT3
	deadend,		//PIO INT2
	deadend,		//PIO INT1
	deadend			//PIO INT0
};

void bootstrap(void) {
//Set STKALIGN in NVIC. Not stritly necessary, but good to do. TODO: Make more readable (i.e. memorymap.h definitions)
#define NVIC_CCR ((volatile unsigned long *)(0xE000ED14))
	*NVIC_CCR = *NVIC_CCR | 0x200; 			

	SYSCON_InitCore72MHzFromExternal12MHz();	// let there be speed
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_GPIO | SYSCON_SYSAHBCLKCTRL_IOCON; // Enable common clocks: GPIO and IOCON

	main();
	while (true) {
		waitForInterrupt();
	}
}

void deadend(void) {
	while(1);
}

