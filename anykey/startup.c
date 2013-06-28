#include "anykey.h"

void bootstrap(void);	//bootstrap that will call main later
void deadend(void);	//neverending handler

/* These variables are used to pass memory locations from the linker script to our code. */
extern uint8_t _LD_STACK_TOP;
extern uint8_t _LD_END_OF_TEXT;
extern uint8_t _LD_START_OF_DATA;
extern uint8_t _LD_END_OF_DATA;
extern uint8_t _LD_END_OF_BSS;

/* we define some standard handler names here - they all default to deadend but may be changed by implementing a real function with that name. So if they are triggered but undefined, we'll just stop. DEFAULT_IMP defines a weak alias. */

#define DEFAULTS_TO(func) __attribute__ ((weak, alias (#func)))

void main(void);// DEFAULTS_TO(deadend);
void systick(void) DEFAULTS_TO(deadend);
void usb_fiq_handler(void) DEFAULTS_TO(deadend);
void usb_irq_handler(void) DEFAULTS_TO(deadend);
void ct16b0_handler(void) DEFAULTS_TO(deadend);
void ct16b1_handler(void) DEFAULTS_TO(deadend);
void ct32b0_handler(void) DEFAULTS_TO(deadend);
void ct32b1_handler(void) DEFAULTS_TO(deadend);
void gpio0_handler(void) DEFAULTS_TO(deadend);
void gpio1_handler(void) DEFAULTS_TO(deadend);
void gpio2_handler(void) DEFAULTS_TO(deadend);
void gpio3_handler(void) DEFAULTS_TO(deadend);
void i2c_handler(void) DEFAULTS_TO(deadend);

/* The vector table - contains the initial stack pointer and
 pointers to boot code as well as interrupt and fault handler pointers.
 The processor will expect this to be located at address 0x0, so
 we put it into a separate linker section. */
__attribute__ ((section(".vectors")))



const VECTOR_TABLE vtable = {
	&_LD_STACK_TOP,          //Stack top
	bootstrap,               //boot code
	deadend,                 //NMI
	deadend,                 //Hard fault
	deadend,                 //Memory protection unit fault handler
	deadend,                 //Bus fault handler
	deadend,                 //Usage fault handler
	deadend,                 //RESERVED1
	deadend,                 //RESERVED2
	deadend,                 //Reserved for CRC checksum
	deadend,                 //RESERVED4
	deadend,                 //SVCall handler
	deadend,                 //Debug monitor handler
	deadend,                 //RESERVED5
	deadend,                 //PendSV handler
	systick,                 //The SysTick handler
	deadend,                 //PIO0_0  Wakeup
	deadend,                 //PIO0_1  Wakeup
	deadend,                 //PIO0_2  Wakeup
	deadend,                 //PIO0_3  Wakeup
	deadend,                 //PIO0_4  Wakeup
	deadend,                 //PIO0_5  Wakeup
	deadend,                 //PIO0_6  Wakeup
	deadend,                 //PIO0_7  Wakeup
	deadend,                 //PIO0_8  Wakeup
	deadend,                 //PIO0_9  Wakeup
	deadend,                 //PIO0_10  Wakeup
	deadend,                 //PIO0_11  Wakeup
	deadend,                 //PIO1_0  Wakeup
	deadend,                 //PIO1_1  Wakeup
	deadend,                 //PIO1_2  Wakeup
	deadend,                 //PIO1_3  Wakeup
	deadend,                 //PIO1_4  Wakeup
	deadend,                 //PIO1_5  Wakeup
	deadend,                 //PIO1_6  Wakeup
	deadend,                 //PIO1_7  Wakeup
	deadend,                 //PIO1_8  Wakeup
	deadend,                 //PIO1_9  Wakeup
	deadend,                 //PIO1_10  Wakeup
	deadend,                 //PIO1_11  Wakeup
	deadend,                 //PIO2_0  Wakeup
	deadend,                 //PIO2_1  Wakeup
	deadend,                 //PIO2_2  Wakeup
	deadend,                 //PIO2_3  Wakeup
	deadend,                 //PIO2_4  Wakeup
	deadend,                 //PIO2_5  Wakeup
	deadend,                 //PIO2_6  Wakeup
	deadend,                 //PIO2_7  Wakeup
	deadend,                 //PIO2_8  Wakeup
	deadend,                 //PIO2_9  Wakeup
	deadend,                 //PIO2_10  Wakeup
	deadend,                 //PIO2_11  Wakeup
	deadend,                 //PIO3_0  Wakeup
	deadend,                 //PIO3_1  Wakeup
	deadend,                 //PIO3_2  Wakeup
	deadend,                 //PIO3_3  Wakeup
	i2c_handler,             //I2C
	ct16b0_handler,          //16-bit Timer 0 handler
	ct16b1_handler,          //16-bit Timer 1 handler
	ct32b0_handler,          //32-bit Timer 0 handler
	ct32b1_handler,          //32-bit Timer 1 handler
	deadend,                 //SSP
	deadend,                 //UART
	usb_irq_handler,         //USB IRQ
	usb_fiq_handler,         //USB FIQ
	deadend,                 //ADC
	deadend,                 //WDT
	deadend,                 //BOD
	deadend,                 //Flash
	gpio3_handler,           //PIO INT3
	gpio2_handler,           //PIO INT2
	gpio1_handler,           //PIO INT1
	gpio0_handler            //PIO INT0
};

void bootstrap(void) {

	//Set STKALIGN in NVIC. Not stritly necessary, but good to do. TODO: Make more readable (i.e. memorymap.h definitions)
#define NVIC_CCR ((volatile unsigned long *)(0xE000ED14))
	*NVIC_CCR = *NVIC_CCR | 0x200;

	// turn up the speed
	SYSCON_InitCore72MHzFromExternal12MHz();	

	//copy initial values of variables (non-const globals and static variables) from FLASH to RAM
	uint8_t* mirror = &_LD_END_OF_TEXT; //copy from here
	uint8_t* ram = &_LD_START_OF_DATA;	//copy to here
	while (ram < (&_LD_END_OF_DATA)) *(ram++) = *(mirror++);

	//set uninitialized globals (and globals initialized to zero) to zero
	while (ram < (&_LD_END_OF_BSS)) *(ram++) = 0;

	//turn on power for some common peripherals (IO, IOCON)
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_GPIO | SYSCON_SYSAHBCLKCTRL_IOCON; // Enable common clocks: GPIO and IOCON

	//jump into main user code (which should setup needed timers and interrupts or not return at all)
	main();

	//after main, sleep until an interrupt occurs
	while (true) {
		waitForInterrupt();
	}
}

/* default handler for unimplemented interrupts or faults. Stay here for debugger to pick up (once we have a debugger) */
void deadend(void) {
	while(1);
}

