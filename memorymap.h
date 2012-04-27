#ifndef _MEMORYMAP_
#define _MEMORYMAP_

/* Some definitions for registers memory mapping. All just resolve to volatile. This way, allowed usage can be seen in the typedefs. HW_WO: Write only, HW_RO: Read only, HW_RW: Read/Write, HW_RS: Reserved (do not access), HW_UU: Unused (padding etc.) */

typedef volatile uint32_t HW_WO;
typedef volatile uint32_t HW_RO;
typedef volatile uint32_t HW_RW;
typedef volatile uint32_t HW_RS;
typedef volatile uint32_t HW_UU;


/* -----------------------------------
   --- GPIO --------------------------
   -----------------------------------

General Purpose Input/Output (GPIO) registers. LPC1343 organizes GPIO pins in 4 ports of 12 bits each. They can be accessed by GPIO[<bank_index>].<field> */

typedef struct {
	HW_RW MASKED_DATA[0xfff];	//bit masked access to pins
	HW_RW DATA;			//all 12 pins
	HW_RS RESERVED1[0x1000];	//reserved (byte offsets 0x4000-0x7fff)
	HW_RW DIR;			//direction bitmap. 0=in, 1=out
	HW_RW IS;			//interrupt sense bitmap. 0=edge, 1=level
	HW_RW IBE;			//interrupt both edges bitmap. 0=see IEV, 1=both edges trigger
	HW_RW IEV;			//interrupt event bitmap. 0=falling edge, 1=rising edge
	HW_RW IE;			//interrupt mask bitmap. 0=interrupt off, 1=on
	HW_RO RIS;			//raw interrupt state bitmap, not masked by IE
	HW_RO MIS;			//masked interrupt state bitmap, masked by IE
	HW_WO IC;			//interrupt clear bitmap. Write 1 to clear edge interrupt mask
	HW_RS RESERVED2[0x3f8];		//reserved (byte offsets 0x8020 - 0x8fff)
	HW_UU UNUSED[0x1c00];		//padding to next GPIO bank (byte offsets 0x9000 to 0xffff)
} GPIO_STRUCT;

#define GPIO ((GPIO_STRUCT*)(0x50000000))

/* -----------------------------------
   --- IOCON -------------------------
   -----------------------------------

IO configuration. Functions are quite a mess...

*/

typedef struct {
	HW_RW PIO2_6;
	HW_RS RESERVED1;
	HW_RW PIO2_0;
	HW_RW RESET_PIO0_0;
	HW_RW PIO0_1;
	HW_RW PIO1_8;
	HW_RS RESERVED2;
	HW_RW PIO0_2;
	HW_RW PIO2_7;
	HW_RW PIO2_8;
	HW_RW PIO2_1;
	HW_RW PIO0_3;
	HW_RW PIO0_4;
	HW_RW PIO0_5;
	HW_RW PIO1_9;
	HW_RW PIO3_4;
	HW_RW PIO2_4;
	HW_RW PIO2_5;
	HW_RW PIO3_5;
	HW_RW PIO0_6;
	HW_RW PIO0_7;
	HW_RW PIO2_9;
	HW_RW PIO2_10;
	HW_RW PIO2_2;
	HW_RW PIO0_8;
	HW_RW PIO0_9;
	HW_RW SWCLK_PIO0_10;
	HW_RW PIO1_10;
	HW_RW PIO2_11;
	HW_RW R_PIO0_11;
	HW_RW R_PIO1_0;
	HW_RW R_PIO1_1;
	HW_RW R_PIO1_2;
	HW_RW PIO3_0;
	HW_RW PIO3_1;
	HW_RW PIO2_3;
	HW_RW SWDIO_PIO1_3;
	HW_RW PIO1_4;
	HW_RW PIO1_11;
	HW_RW PIO3_2;
	HW_RW PIO1_5;
	HW_RW PIO1_6;
	HW_RW PIO1_7;
	HW_RW PIO3_3;
	HW_RW SCK0_LOC;
	HW_RW DSR_LOC;
	HW_RW DCD_LOC;
	HW_RW RI_LOC;
} IOCON_STRUCT;

#define IOCON ((IOCON_STRUCT*)(0x40044000))

/* -----------------------------------
   --- SYSCON  -----------------------
   -----------------------------------

System configuration block. This area contains oscilltors, power management and clock generation. */

typedef struct {
	HW_RW SYSMEMREMAP;		//System memory remap. Bit1:0. 00:boot loader, 01:user ram, 1x:flash
	HW_RW PRESETCTRL;		//Periperal reset. Bit 0:SSP, 1: I2C. Value 0-enable, 1-de-assert
	HW_RW SYSPLLCTRL;		//System PLL divider. Bit 4:0:FB divider (1..32). Bit 6:5: Post divider(1/2/4/8)
	HW_RO SYSPLLSTAT;		//System PLL lock. Bit 0. 0:Not locked, 1:locked
	HW_RW USBPLLCTRL;		//USB PLL divider. Bit 4:0:FB divider (1..32). Bit 6:5: Post divider(1/2/4/8)
	HW_RO USBPLLSTAT;		//USB PLL lock. Bit 0. 0:Not locked, 1:locked
	HW_RS RESERVED1[2];
	HW_RW SYSOSCCTRL;		//Sys oscillator control. Bit 0: bypass, bit 1: range. 0:1-20MHz, 1:15-25MHz
	HW_RW WDTOSCCTRL;		//Watchdog oscillator. Bit4:0: divider(2..64), Bit8:5 Freq (0.5..3.4MHz)
	HW_RW IRCCTRL;			//Internal oscillator trim. Bit7:0.
	HW_RS RESERVED2;
	HW_RO SYSRESSTAT;		//Sys reset. Bit0: POR, Bit1: Ext Pin, Bit2: WDT, Bit 3:BOD, Bit4:Sys Reset
	HW_RS RESERVED3[3];		
	HW_RW SYSPLLCLKSEL;		//Sys PLL source. Bit1:0: 00:Internal RC, 01:Sys oscillator
	HW_RW SYSPLLCLKUEN;		//Sys PLL update. To trigger written SYSPLLCLKSEL, write 0 then 1
	HW_RW USBPLLCLKSEL;		//USB PLL source. Bit1:0: 00:Internal RC, 01:Sys oscillator
	HW_RW USBPLLCLKUEN;		//USB PLL update. To trigger written USBPLLCLKSEL, write 0 then 1
	HW_RS RESERVED4[8];
	HW_RW MAINCLKSEL;		//Main clock source. Bit1:0. 00:IRC, 01: Sys PLL in, 10: WDT, 11: Sys PLL out
	HW_RW MAINCLKUEN;		//Main clock update. To trigger written MAINCLKSEL, write 0 then 1
	HW_RW SYSAHBCLKDIV;		//AHB Clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RS RESERVED5;
	HW_RW SYSAHBCLKCTRL;		//AHB clock ctrl, enable bits for various peripheral clocks. List in manual.
	HW_RS RESERVED6[4];
	HW_RW SSPCLKDIV;		//SSP clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RW UARTCLKDIV;		//UART clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RS RESERVED7[4];
	HW_RW TRACECLKDIV;		//Trace clock divider. Bit7:0: 0:Trace clock off, 1..255: divide by 1..255
	HW_RW SYSTICKCLKDIV;		//SYSTICK clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RS RESERVED8[3];
	HW_RW USBCLKSEL;		//USB clock source. Bit1:0. 00:USB PLL out, 01: Main clock
	HW_RW USBCLKUEN;		//USB clock update. To trigger written USBCLKSEL, write 0 then 1
	HW_RW USBCLKDIV;		//USB clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RS RESERVED9;
	HW_RW WDTCLKSEL;		//Watchdog source select. Bit1:0: 00:IRC, 01:Main clock, 10:WD osc
	HW_RW WDTCLKUEN;		//Watchdog source update. To trigger written WDTCLKSEL, write 0 then 1
	HW_RW WDTCLKDIV;		//Watchdog clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RS RESERVED10;
	HW_RW CLKOUTCLKSEL;		//Output clock source. Bit1:0: 00:IRC, 01:Sys osc, 10:WD osc, 11:Main clk
	HW_RW CLKOUTUEN;		//Output clock source update. To trigger written CLKOUTCLKSEL, write 0 then 1
	HW_RW CLKOUTDIV;		//Out clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RS RESERVED11[5];
	HW_RO PIOPORCAP0;		//PIO pin state at Power-on reset
	HW_RO PIOPORCAP1;		//PIO pin state at Power-on reset
	HW_RS RESERVED12[18];
	HW_RW BODCTRL;			//Brownout control. Bit1:0:Reset V, Bit3:2:Interrupt V, Bit4:Enable BOD reset
	HW_RS RESERVED13;
	HW_RW SYSTCKCAL;		//Systick timer calibration
	HW_RS RESERVED14[41];
	HW_RW STARTAPRP0;		//Start logic edge control bitmap. Rising or falling edge of PIO
	HW_RW STARTERP0;		//Start logic signal enable bitmap. PIO interrupt enable mask
	HW_WO STARTRSRP0CLR;		//Start logic reset bitmap. Write 1 to reset start signal
	HW_RO STARTSRP0;		//Start logic status bitmap (start signals pending from resp. pins)
	HW_RW STARTAPRP1;		//See STARTAPRP0, other PIO pins
	HW_RW STARTERP1;		//See STARTERP1, other PIO pins
	HW_WO STARTRSRP1CLR;		//See STARTRSRP1CLR, other PIO pins
	HW_RO STARTSRP1;		//See STARTSRP1, other PIO pins
	HW_RS RESERVED15[4];
	HW_RW PDSLEEPCFG;		//Deep-sleep config. Mask controls what to power off in deep sleep.
	HW_RW PDAWAKECFG;		//Wakeup config. Mask controls what to power on after deep sleep wakeup
	HW_RW PDRUNCFG;			//Power-down config. Mask controls what to power off in power-down.
	HW_RS RESERVED16[110];
	HW_RO DEVICE_ID;		//Device ID - Chip model.
} SYSCON_STRUCT;

#define SYSCON ((SYSCON_STRUCT*)(0x40048000))

/* -----------------------------------
   --- SYSTICK  ----------------------
   -----------------------------------

Systick registers control the behaviour of the systick timer. Note that parts of the systick functionality are also in SYSCON. */

typedef struct {
	HW_UU UNUSED[4];		//Padding
	HW_RW CTRL;			//Control and status register
	HW_RW LOAD;			//Reload value
	HW_RW VAL;			//Current value
	HW_RO CALIB;			//Calibration
} SYSTICK_STRUCT;

#define SYSTICK ((SYSTICK_STRUCT*)(0xe000e000))

/* -----------------------------------
   --- VECTOR TABLE  -----------------
   -----------------------------------

The vector table specifies the boot stack position, the boot address and interrupt function pointers. The initial vector table is located in FLASH at 0x00000000, providing the entry point in the boot process. It can be relocated to RAM. */

typedef struct {
	void* STACK_ENTRY;		//Initial Stack pointer
  	void* BOOT_HANDLER;		//Reset handler
	void* NMI_HANDLER;		//NMI handler
	void* HARDFAULT_HANDLER;	//Hard fault handler
	void* MPUFAULT_HANDLER;		//Memory protection unit fault handler
	void* BUSFAULT_HANDLER;		//Bus fault handler
	void* USAGEFAULT_HANDLER;	//Usage fault handler
	void* RESERVED1;
	void* RESERVED2;
	void* RESERVED3;		//Reserved for CRC checksum
	void* RESERVED4;
	void* SVCALL_HANDLER;		//SVCall handler
	void* DEBUGMON_HANDLER;		//Debug monitor handler
	void* RESERVED5;
	void* PENDSV_HANDLER;		//PendSV handler
	void* SYSTICK_HANDLER;		//The SysTick handler
	void* PIO0_0_WAKEUP_HANDLER;	//PIO0_0  Wakeup
	void* PIO0_1_WAKEUP_HANDLER;	//PIO0_1  Wakeup
	void* PIO0_2_WAKEUP_HANDLER;	//PIO0_2  Wakeup
	void* PIO0_3_WAKEUP_HANDLER;	//PIO0_3  Wakeup
	void* PIO0_4_WAKEUP_HANDLER;	//PIO0_4  Wakeup
	void* PIO0_5_WAKEUP_HANDLER;	//PIO0_5  Wakeup
	void* PIO0_6_WAKEUP_HANDLER;	//PIO0_6  Wakeup
	void* PIO0_7_WAKEUP_HANDLER;	//PIO0_7  Wakeup
	void* PIO0_8_WAKEUP_HANDLER;	//PIO0_8  Wakeup
	void* PIO0_9_WAKEUP_HANDLER;	//PIO0_9  Wakeup
	void* PIO0_10_WAKEUP_HANDLER;	//PIO0_10  Wakeup
	void* PIO0_11_WAKEUP_HANDLER;	//PIO0_11  Wakeup
	void* PIO1_0_WAKEUP_HANDLER;	//PIO1_0  Wakeup
	void* PIO1_1_WAKEUP_HANDLER;	//PIO1_1  Wakeup
	void* PIO1_2_WAKEUP_HANDLER;	//PIO1_2  Wakeup
	void* PIO1_3_WAKEUP_HANDLER;	//PIO1_3  Wakeup
	void* PIO1_4_WAKEUP_HANDLER;	//PIO1_4  Wakeup
	void* PIO1_5_WAKEUP_HANDLER;	//PIO1_5  Wakeup
	void* PIO1_6_WAKEUP_HANDLER;	//PIO1_6  Wakeup
	void* PIO1_7_WAKEUP_HANDLER;	//PIO1_7  Wakeup
	void* PIO1_8_WAKEUP_HANDLER;	//PIO1_8  Wakeup
	void* PIO1_9_WAKEUP_HANDLER;	//PIO1_9  Wakeup
	void* PIO1_10_WAKEUP_HANDLER;	//PIO1_10  Wakeup
	void* PIO1_11_WAKEUP_HANDLER;	//PIO1_11  Wakeup
	void* PIO2_0_WAKEUP_HANDLER;	//PIO2_0  Wakeup
	void* PIO2_1_WAKEUP_HANDLER;	//PIO2_1  Wakeup
	void* PIO2_2_WAKEUP_HANDLER;	//PIO2_2  Wakeup
	void* PIO2_3_WAKEUP_HANDLER;	//PIO2_3  Wakeup
	void* PIO2_4_WAKEUP_HANDLER;	//PIO2_4  Wakeup
	void* PIO2_5_WAKEUP_HANDLER;	//PIO2_5  Wakeup
	void* PIO2_6_WAKEUP_HANDLER;	//PIO2_6  Wakeup
	void* PIO2_7_WAKEUP_HANDLER;	//PIO2_7  Wakeup
	void* PIO2_8_WAKEUP_HANDLER;	//PIO2_8  Wakeup
	void* PIO2_9_WAKEUP_HANDLER;	//PIO2_9  Wakeup
	void* PIO2_10_WAKEUP_HANDLER;	//PIO2_10  Wakeup
	void* PIO2_11_WAKEUP_HANDLER;	//PIO2_11  Wakeup
	void* PIO3_0_WAKEUP_HANDLER;	//PIO3_0  Wakeup
	void* PIO3_1_WAKEUP_HANDLER;	//PIO3_1  Wakeup
	void* PIO3_2_WAKEUP_HANDLER;	//PIO3_2  Wakeup
	void* PIO3_3_WAKEUP_HANDLER;	//PIO3_3  Wakeup
	void* I2C_HANDLER;		//I2C
	void* TIMER16_0_HANDLER;	//16-bit Timer 0 handler
	void* TIMER16_1_HANDLER;	//16-bit Timer 1 handler
	void* TIMER32_0_HANDLER;	//32-bit Timer 0 handler
	void* TIMER32_1_HANDLER;	//32-bit Timer 1 handler
	void* SSP_HANDLER;		//SSP
	void* UART_HANDLER;		//UART
	void* USB_IRQ_HANDLER;		//USB IRQ
	void* USB_FIQ_HANDLER;		//USB FIQ
	void* ADC_HANDLER;		//ADC
	void* WDT_HANDLER;		//WDT
	void* BOD_HANDLER;		//BOD
	void* FLASH_HANDLER;		//Flash
	void* PIOINT3_HANDLER;		//PIO INT3
	void* PIOINT2_HANDLER;		//PIO INT2
	void* PIOINT1_HANDLER;		//PIO INT1
	void* PIOINT0_HANDLER;		//PIO INT0
} VECTOR_TABLE;


#endif