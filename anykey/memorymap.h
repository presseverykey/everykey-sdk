#ifndef _MEMORYMAP_
#define _MEMORYMAP_

#include "types.h"

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

// GPIO bank bases can be accessed via GPIO[i]
#define GPIO ((GPIO_STRUCT*)(0x50000000))

/* -----------------------------------
   --- IOCON -------------------------
   -----------------------------------

IO configuration. Functions are quite a mess... */

typedef struct {
	HW_RW PIO2_6;
	HW_RS RESERVED1;
	HW_RW PIO2_0;
	HW_RW PIO0_0;
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
	HW_RW PIO0_10;
	HW_RW PIO1_10;
	HW_RW PIO2_11;
	HW_RW PIO0_11;
	HW_RW PIO1_0;
	HW_RW PIO1_1;
	HW_RW PIO1_2;
	HW_RW PIO3_0;
	HW_RW PIO3_1;
	HW_RW PIO2_3;
	HW_RW PIO1_3;
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

typedef enum IOCON_IO_PULL_MODE {
	IOCON_IO_PULL_NONE = 0x00,
	IOCON_IO_PULL_DOWN = 0x08,
	IOCON_IO_PULL_UP = 0x10,
	IOCON_IO_PULL_REPEAT = 0x18
} IOCON_IO_PULL_MODE;

typedef enum IOCON_IO_HYSTERESIS_MODE {
	IOCON_IO_HYSTERESIS_OFF = 0x00,
	IOCON_IO_HYSTERESIS_ON = 0x20
} IOCON_IO_HYSTERESIS_MODE;

typedef enum IOCON_IO_FUNC {
	IOCON_IO_FUNC_PIO0_1_PIO    = 0x00,
	IOCON_IO_FUNC_PIO0_1_CLK    = 0x01,
	IOCON_IO_FUNC_PIO0_1_TIMER  = 0x02,
	IOCON_IO_FUNC_PIO0_1_USBTG  = 0x03,

	IOCON_IO_FUNC_PIO0_2_PIO    = 0x00,
	IOCON_IO_FUNC_PIO0_2_SSP    = 0x01,
	IOCON_IO_FUNC_PIO0_2_TIMER  = 0x02,

	IOCON_IO_FUNC_PIO0_4_PIO    = 0x00, // open drain
	IOCON_IO_FUNC_PIO0_4_SCL    = 0x01,

	IOCON_IO_FUNC_PIO0_5_PIO    = 0x00, // open drain
	IOCON_IO_FUNC_PIO0_5_SDA    = 0x01,

	IOCON_IO_FUNC_PIO0_6_PIO    = 0x00, 
	IOCON_IO_FUNC_PIO0_6_USBCNT = 0x01,
	IOCON_IO_FUNC_PIO0_6_SCK    = 0x02, // s. IOCON_SCK0_LOC

	IOCON_IO_FUNC_PIO0_7_PIO    = 0x00, 
	IOCON_IO_FUNC_PIO0_7_CTS    = 0x01,

	IOCON_IO_FUNC_PIO0_8_PIO    = 0x00,
	IOCON_IO_FUNC_PIO0_8_SSP    = 0x01,
	IOCON_IO_FUNC_PIO0_8_TMR    = 0x02,
	
	IOCON_IO_FUNC_PIO0_9_PIO    = 0x00,
	IOCON_IO_FUNC_PIO0_9_SSP    = 0x01,
	IOCON_IO_FUNC_PIO0_9_TMR    = 0x02,
	IOCON_IO_FUNC_PIO0_9_SWD    = 0x03,

	IOCON_IO_FUNC_PIO0_10_SWD   = 0x00,
	IOCON_IO_FUNC_PIO0_10_PIO   = 0x01,
	IOCON_IO_FUNC_PIO0_10_SSP   = 0x02,
	IOCON_IO_FUNC_PIO0_10_TMR   = 0x03,
	
	IOCON_IO_FUNC_PIO0_11_R     = 0x00,  // Reserved
	IOCON_IO_FUNC_PIO0_11_PIO   = 0x01,
	IOCON_IO_FUNC_PIO0_11_ADC   = 0x02,
	IOCON_IO_FUNC_PIO0_11_TMR   = 0x03,

	IOCON_IO_FUNC_PIO1_0_PIO    = 0x01,
	IOCON_IO_FUNC_PIO1_0_ADC    = 0x02,
	IOCON_IO_FUNC_PIO1_0_TMR    = 0x03,
	
	IOCON_IO_FUNC_PIO1_1_R      = 0x00, // RESERVED
	IOCON_IO_FUNC_PIO1_1_PIO    = 0x01,
	IOCON_IO_FUNC_PIO1_1_ADC    = 0x02,
	IOCON_IO_FUNC_PIO1_1_TMR    = 0x03,

	IOCON_IO_FUNC_PIO1_2_R      = 0x00, // RESERVED
	IOCON_IO_FUNC_PIO1_2_PIO    = 0x01,
	IOCON_IO_FUNC_PIO1_2_ADC    = 0x02,
	IOCON_IO_FUNC_PIO1_2_TMR    = 0x03,

	IOCON_IO_FUNC_PIO1_3_SWD    = 0x00,
	IOCON_IO_FUNC_PIO1_3_PIO    = 0x01,
	IOCON_IO_FUNC_PIO1_3_ADC    = 0x02,
	IOCON_IO_FUNC_PIO1_3_TMR    = 0x03,

	IOCON_IO_FUNC_PIO1_4_PIO    = 0x00,
	IOCON_IO_FUNC_PIO1_4_ADC    = 0x01,
	IOCON_IO_FUNC_PIO1_4_TMR    = 0x02,

	IOCON_IO_FUNC_PIO1_5_PIO    = 0x00,
	IOCON_IO_FUNC_PIO1_5_RTS    = 0x01,
	IOCON_IO_FUNC_PIO1_5_TMR    = 0x02,

	IOCON_IO_FUNC_PIO1_6_PIO    = 0x00,
	IOCON_IO_FUNC_PIO1_6_RXD    = 0x01,
	IOCON_IO_FUNC_PIO1_6_TMR    = 0x02,

	IOCON_IO_FUNC_PIO1_7_PIO    = 0x00,
	IOCON_IO_FUNC_PIO1_7_TXD    = 0x01,
	IOCON_IO_FUNC_PIO1_7_TMR    = 0x02,


	IOCON_IO_FUNC_PIO1_10_PIO   = 0x00,
	IOCON_IO_FUNC_PIO1_10_ADC   = 0x01,
	IOCON_IO_FUNC_PIO1_10_TMR   = 0x02,

	IOCON_IO_FUNC_PIO1_11_PIO   = 0x00,
	IOCON_IO_FUNC_PIO1_11_ADC   = 0x01,
	
	IOCON_IO_FUNC_PIO2_0_PIO   = 0x00,
	IOCON_IO_FUNC_PIO2_0_DTR   = 0x01,
	IOCON_IO_FUNC_PIO2_0_SSEL  = 0x02,

	IOCON_IO_FUNC_PIO2_1_PIO   = 0x00,
	IOCON_IO_FUNC_PIO2_1_DSR   = 0x01,
	IOCON_IO_FUNC_PIO2_1_SCK   = 0x02,

	IOCON_IO_FUNC_PIO2_2_PIO   = 0x00,
	IOCON_IO_FUNC_PIO2_2_DCD   = 0x01,
	IOCON_IO_FUNC_PIO2_2_MISO  = 0x02,

	IOCON_IO_FUNC_PIO2_3_PIO   = 0x00,
	IOCON_IO_FUNC_PIO2_3_RI    = 0x01,
	IOCON_IO_FUNC_PIO2_3_MOSI  = 0x02,

	IOCON_IO_FUNC_PIO2_11_PIO   = 0x00,
	IOCON_IO_FUNC_PIO2_11_SSP   = 0x01	//Wrong value (0x02) in older versions of the user manual
	
} IOCON_IO_FUNC;

typedef enum IOCON_IO_ADMODE {
	IOCON_IO_ADMODE_ANALOG  = 0x00,
	IOCON_IO_ADMODE_DIGITAL = 0x80
} IOCON_IO_ADMODE;

typedef enum IOCON_SCK0_LOC_VALUES {
	IOCON_SCK0_LOC_PIO0_10 = 0x00,
	IOCON_SCK0_LOC_PIO2_11 = 0x01,
	IOCON_SCK0_LOC_PIO0_6 = 0x02
} IOCON_SCK0_LOC_VALUES;

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
	HW_RW SSP0CLKDIV;		//SSP0 clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RW UARTCLKDIV;		//UART clock divider. Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RW SSP1CLKDIV;		//SSP1 clock divider (only on LPC1313FBD48). Bit7:0: 0:disable, 1..255: divide by 1..255
	HW_RS RESERVED7[3];
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

typedef enum SYSCON_SYSAHBCLKCTRL_BITS {
	SYSCON_SYSAHBCLKCTRL_SYS        = 0x00001,
	SYSCON_SYSAHBCLKCTRL_ROM        = 0x00002,
	SYSCON_SYSAHBCLKCTRL_RAM        = 0x00004,
	SYSCON_SYSAHBCLKCTRL_FLASHREG   = 0x00008,
	SYSCON_SYSAHBCLKCTRL_FLASHARRAY = 0x00010,
	SYSCON_SYSAHBCLKCTRL_I2C        = 0x00020,
	SYSCON_SYSAHBCLKCTRL_GPIO       = 0x00040,
	SYSCON_SYSAHBCLKCTRL_CT16B0     = 0x00080,
	SYSCON_SYSAHBCLKCTRL_CT16B1     = 0x00100,
	SYSCON_SYSAHBCLKCTRL_CT32B0     = 0x00200,
	SYSCON_SYSAHBCLKCTRL_CT32B1     = 0x00400,
	SYSCON_SYSAHBCLKCTRL_SSP        = 0x00800,
	SYSCON_SYSAHBCLKCTRL_UART       = 0x01000,
	SYSCON_SYSAHBCLKCTRL_ADC        = 0x02000,
	SYSCON_SYSAHBCLKCTRL_USB_REG    = 0x04000,
	SYSCON_SYSAHBCLKCTRL_WDT        = 0x08000,
	SYSCON_SYSAHBCLKCTRL_IOCON      = 0x10000,
	SYSCON_SYSAHBCLKCTRL_SSP1       = 0x40000
} SYSCON_SYSAHBCLKCTRL_BITS;

/* may be used with PDRUNCFG and PDAWAKECFG registers */
typedef enum SYSCON_PD_BITS {
	SYSCON_IRCOUT_PD     = 0x0001,
	SYSCON_IRC_PD        = 0x0002,
	SYSCON_FLASH_PD      = 0x0004,
	SYSCON_BOD_PD        = 0x0008,
	SYSCON_ADC_PD        = 0x0010,
	SYSCON_SYSOSC_PD     = 0x0020,
	SYSCON_WDTOSC_PD     = 0x0040,
	SYSCON_SYSPLL_PD     = 0x0080,
	SYSCON_USBPLL_PD     = 0x0100,
	SYSCON_USBPAD_PD     = 0x0400,
	SYSCON_PD_ALWAYS_SET = 0xe800
} SYSCON_PD_BITS;

typedef enum SYSCON_PRESETCTRL_BITS {
	SYSCON_PRESETCTRL_SSP0_RST_N = 0x01,
	SYSCON_PRESETCTRL_I2C_RST_N = 0x02,		//NOTE: This is taken from the UM, seems to be wrong
	SYSCON_PRESETCTRL_SSP1_RST_N = 0x04
} SYSCON_PRESETCTRL_BITS;

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
   --- USB  --------------------------
   -----------------------------------

USB peripheral access. */

typedef struct {
	HW_RO DEVINTST;			//Device interrupt status
	HW_RW DEVINTEN;			//Device interrupt enable
	HW_WO DEVINTCLR;		//Device interrupt clear
	HW_WO DEVINTSET;		//Device interrupt set
	HW_WO CMDCODE;			//SIE Command code / data write
	HW_RO CMDDATA;			//SIE Command data / data read
	HW_RO RXDATA;			//Receive data
	HW_WO TXDATA;			//Transmit data
	HW_RO RXPLEN;			//Receive packet length
	HW_WO TXPLEN;			//Transmit packet length
	HW_RW CTRL;			//Control
	HW_WO DEVFIQSEL;		//Device FIQ select
} USB_STRUCT;

typedef enum {
	USB_CMDCODE_PHASE_WRITE = 0x100,
	USB_CMDCODE_PHASE_READ = 0x200,
	USB_CMDCODE_PHASE_COMMAND = 0x500,
} USB_CMDCODE_PHASE;

//interrupt bits. Can be used with DEVINTST, DEVINTEN, DEVINTCLR and DEVINTSET.
typedef enum {
	USB_DEVINT_FRAME    = 0x0001,
	USB_DEVINT_EP0      = 0x0002,
	USB_DEVINT_EP1      = 0x0004,
	USB_DEVINT_EP2      = 0x0008,
	USB_DEVINT_EP3      = 0x0010,
	USB_DEVINT_EP4      = 0x0020,
	USB_DEVINT_EP5      = 0x0040,
	USB_DEVINT_EP6      = 0x0080,
	USB_DEVINT_EP7      = 0x0100,
	USB_DEVINT_DEV_STAT = 0x0200,
	USB_DEVINT_CC_EMPTY = 0x0400,
	USB_DEVINT_CD_FULL  = 0x0800,
	USB_DEVINT_RXENDPKT = 0x1000,
	USB_DEVINT_TXENDPKT = 0x2000
} USB_DEVINT_TARGET;

//USB SIE Commands
typedef enum USB_SIE_CommandID {
	USB_SIE_CMD_SetAddress             = 0xd0,				//device - write 1 byte
	USB_SIE_CMD_ConfigureDevice        = 0xd8,				//device - write 1 byte
	USB_SIE_CMD_SetMode                = 0xf3,				//device - write 1 byte
	USB_SIE_CMD_ReadInterruptStatus    = 0xf4,			  //device - read 1 or 2 bytes
	USB_SIE_CMD_ReadCurrentFrameNumber = 0xf5,		    //device - read 1 or 2 bytes
	USB_SIE_CMD_ReadChipID             = 0xfd,				//device - read 2 bytes
	USB_SIE_CMD_GetSetDeviceStatus     = 0xfe,			  //device - write 1 byte (set) or read 1 byte (get)
	USB_SIE_CMD_GetErrorCode           = 0xff,				//device - read 1 byte
	USB_SIE_CMD_SelectEndpoint         = 0x00,				//endpoint 0 - read 1 byte (optional). This is the base address, add the physical EP index (0..9)
	USB_SIE_CMD_SelectEndpointClearInterruptSetStatus  = 0x40,	//endpoint 0 - read 1 byte (select/clear int) or write 1 byte (set endpoint status). This is the base address, add physical EP index (0-9)
	USB_SIE_CMD_ClearBuffer            = 0xf2,			//selected endpoint - read 1 byte (optional)
	USB_SIE_CMD_ValidateBuffer         = 0xfa				//selected endpoint - no read/write
} USB_SIE_CommandID;

//SIE Select Endpoint status bits
typedef enum {
	USB_SELEP_FE     = 0x01,	//FULL/EMPTY (IN: 0=At least one write buffer is empty, OUT: 1=at least one read buffer is full)
	USB_SELEP_ST     = 0x02,	//STALL (endpoint is stalled)
	USB_SELEP_STP    = 0x04,	//SETUP (last received packet was a setup packet)
	USB_SELEP_PO     = 0x08,	//Packet Overwritten (at least one packet was overwritten)
	USB_SELEP_EPN    = 0x10, //Endpoint NAKed
	USB_SELEP_B1FULL = 0x20, //Buffer 1 full
	USB_SELEP_B2FULL = 0x40	//Buffer 2 full
} USB_SELECT_EP_STATUS;

//SIE Set Endpoint status bits
typedef enum {
	USB_EPSTAT_ST     = 0x01,	//Stall (0 = not stalled, 1 = stalled)
	USB_EPSTAT_DA     = 0x10,	//disable (0 = enabled, 1 = disabled)
	USB_EPSTAT_CND_ST = 0x80,	//Conditional stall (0 = unstall both ends, 1 = stall both unless STP is set in SELEP)
} USB_SET_EP_STATUS;

typedef enum {
	USB_CTRL_RD_EN		= 0x01,	//set to read
	USB_CTRL_WR_EN		= 0x02, //set to write
	USB_CTRL_LOG_EP		= 0x04, //logical endpoint number is bit 2..5
} USB_CTRL_VALUES;

typedef enum {
	USB_RXPLEN_DV			= 0x400,
	USB_RXPLEN_LENGTH_MASK	= 0x3ff
} USB_RXPLEN_VALUES;


#define USB ((USB_STRUCT*)(0x40020000))

/* -----------------------------------
   --- NVIC --------------------------
   -----------------------------------

The Nested Vector Interrupt Controller controls interrupts. Note that we use a different base address (0xe000e1000 instead of 0xe000e000) because the first 256 bytes are not used. */

typedef struct {
	HW_RW ISER0;	//Interrupt set enabled
	HW_RW ISER1;
	HW_RW ICER0;	//Interrupt clear enabled
	HW_RW ICER1;
	HW_RW ISPR0;	//Interrupt set pending
	HW_RW ISPR1;
	HW_RW ICPR0;	//Interrupt clear pending
	HW_RW ICPR1;
	HW_RO IABR0;	//Interrupt active
	HW_RO IABR1;	
	HW_RW IPR0;	//Interrupt priority
	HW_RW IPR1;
	HW_RW IPR2;
	HW_RW IPR3;
	HW_RW IPR4;
	HW_RW IPR5;
	HW_RW IPR6;
	HW_RW IPR7;
	HW_RW IPR8;
	HW_RW IPR9;
	HW_RW IPR10;
	HW_RW IPR11;
	HW_RW IPR12;
	HW_RW IPR13;
	HW_RW IPR14;
	HW_WO STIR;	//Software trigger interrupt register
} NVIC_STRUCT;

/* due to the number of interrupts, it doesn't make much sense to write out all masks explicitly - they are split over multiple registers anyway. Instead, they are just numbered. The respective register and bit mask can be obtained by comparison and bit shifting. */

typedef enum NVIC_INTERRUPT_INDEX {
	NVIC_PIO0_0 = 0,
	NVIC_PIO0_1,
	NVIC_PIO0_2,
	NVIC_PIO0_3,
	NVIC_PIO0_4,
	NVIC_PIO0_5,
	NVIC_PIO0_6,
	NVIC_PIO0_7,
	NVIC_PIO0_8,
	NVIC_PIO0_9,
	NVIC_PIO0_10,
	NVIC_PIO0_11,
	NVIC_PIO1_0,
	NVIC_PIO1_1,
	NVIC_PIO1_2,
	NVIC_PIO1_3,
	NVIC_PIO1_4,
	NVIC_PIO1_5,
	NVIC_PIO1_6,
	NVIC_PIO1_7,
	NVIC_PIO1_8,
	NVIC_PIO1_9,
	NVIC_PIO1_10,
	NVIC_PIO1_11,
	NVIC_PIO2_0,
	NVIC_PIO2_1,
	NVIC_PIO2_2,
	NVIC_PIO2_3,
	NVIC_PIO2_4,
	NVIC_PIO2_5,
	NVIC_PIO2_6,
	NVIC_PIO2_7,
	NVIC_PIO2_8,
	NVIC_PIO2_9,
	NVIC_PIO2_10,
	NVIC_PIO2_11,
	NVIC_PIO3_0,
	NVIC_PIO3_1,
	NVIC_PIO3_2,
	NVIC_PIO3_3,
	NVIC_I2C0,
	NVIC_CT16B0,
	NVIC_CT16B1,
	NVIC_CT32B0,
	NVIC_CT32B1,
	NVIC_SSP0,
	NVIC_UART,
	NVIC_USBIRQ,
	NVIC_USBFIQ,
	NVIC_ADC,
	NVIC_WDT,
	NVIC_BOD,
	NVIC_PIO_3 = 53,
	NVIC_PIO_2,
	NVIC_PIO_1,
	NVIC_PIO_0,
	NVIC_SSP1
} NVIC_INTERRUPT_INDEX;

#define NVIC ((NVIC_STRUCT*)0xe000e100)

/* --------------------------------------
   --- Synchronous Serial Port  (SSP) ---
   --------------------------------------
 
 The SSP can be used for SPI, TI serial protocol or microwire. */

typedef struct {
	HW_RW CR0;		//Control register 0
	HW_RW CR1;		//Control register 1
	HW_RW DR;		//Data register
	HW_RO SR;		//Status register
	HW_RW CPSR;		//Clock prescale register
	HW_RW IMSC;		//Interrupt mask set and clear register
	HW_RO RIS;		//Raw interrupt status register
	HW_RO MIS;		//Masked interrupt status register
	HW_WO ICR;		//SSPICR interrupt clear register 
} SSP_STRUCT;


typedef enum SSP_CR0_VALUES {
	SSP_CR0_DSS_4BIT      = 0x03,
	SSP_CR0_DSS_5BIT      = 0x04,
	SSP_CR0_DSS_6BIT      = 0x05,
	SSP_CR0_DSS_7BIT      = 0x06,
	SSP_CR0_DSS_8BIT      = 0x07,
	SSP_CR0_DSS_9BIT      = 0x08,
	SSP_CR0_DSS_10BIT     = 0x09,
	SSP_CR0_DSS_11BIT     = 0x0a,
	SSP_CR0_DSS_12BIT     = 0x0b,
	SSP_CR0_DSS_13BIT     = 0x0c,
	SSP_CR0_DSS_14BIT     = 0x0d,
	SSP_CR0_DSS_15BIT     = 0x0e,
	SSP_CR0_DSS_16BIT     = 0x0f,
	SSP_CR0_FRF_SPI       = 0x00,
	SSP_CR0_FRF_TI        = 0x10,
	SSP_CR0_FRF_MICROWIRE = 0x20,
	SSP_CR0_CPOL_LOW      = 0x00,
	SSP_CR0_CPOL_HIGH     = 0x40,
	SSP_CR0_CPHA_FIRST    = 0x00,
	SSP_CR0_CPHA_SECOND   = 0x80,
	SSP_CR0_SCR_BASE      = 0x100		//Multiply with clock rate-1 (prescaler-out clocks per bit)
} SSP_CR0_VALUES;


typedef enum SSP_CR1_VALUES {
	SSP_CR1_LBM_NORMAL      = 0x00,
	SSP_CR1_LBM_LOOPBACK    = 0x01,
	SSP_CR1_SSE_DISABLE     = 0x00,
	SSP_CR1_SSE_ENABLE      = 0x02,
	SSP_CR1_MS_MASTER       = 0x00,
	SSP_CR1_MS_SLAVE        = 0x04,
	SSP_CR1_SOD             = 0x08
} SSP_CR1_VALUES;

typedef enum SSP_SR_BITS {
	SSP_SR_TFE					= 0x01,
	SSP_SR_TNF					= 0x02,
	SSP_SR_RNE					= 0x04,
	SSP_SR_RFF					= 0x08,
	SSP_SR_BSY					= 0x10
} SSP_SR_BITS;
	
#define SSP0 ((SSP_STRUCT*)0x40040000)
//#define SSP1 ((SSP_STRUCT*)0x40058000) //Not available on LPC1343



/* -----------------------------------
   --- VECTOR TABLE  -----------------
   -----------------------------------

The vector table specifies the boot stack position, the boot address and interrupt function pointers. The initial vector table is located in FLASH at 0x00000000, providing the entry point in the boot process. It can be relocated to RAM. */

typedef struct {
	void* STACK_ENTRY;              //Initial Stack pointer
	void* BOOT_HANDLER;             //Reset handler
	void* NMI_HANDLER;              //NMI handler
	void* HARDFAULT_HANDLER;        //Hard fault handler
	void* MPUFAULT_HANDLER;         //Memory protection unit fault handler
	void* BUSFAULT_HANDLER;         //Bus fault handler
	void* USAGEFAULT_HANDLER;       //Usage fault handler
	void* RESERVED1;
	void* RESERVED2;
	void* RESERVED3;                //Reserved for CRC checksum
	void* RESERVED4;
	void* SVCALL_HANDLER;           //SVCall handler
	void* DEBUGMON_HANDLER;        	//Debug monitor handler
	void* RESERVED5;
	void* PENDSV_HANDLER;           //PendSV handler
	void* SYSTICK_HANDLER;          //The SysTick handler
	void* PIO0_0_WAKEUP_HANDLER;    //PIO0_0  Wakeup
	void* PIO0_1_WAKEUP_HANDLER;    //PIO0_1  Wakeup
	void* PIO0_2_WAKEUP_HANDLER;    //PIO0_2  Wakeup
	void* PIO0_3_WAKEUP_HANDLER;    //PIO0_3  Wakeup
	void* PIO0_4_WAKEUP_HANDLER;    //PIO0_4  Wakeup
	void* PIO0_5_WAKEUP_HANDLER;    //PIO0_5  Wakeup
	void* PIO0_6_WAKEUP_HANDLER;    //PIO0_6  Wakeup
	void* PIO0_7_WAKEUP_HANDLER;    //PIO0_7  Wakeup
	void* PIO0_8_WAKEUP_HANDLER;    //PIO0_8  Wakeup
	void* PIO0_9_WAKEUP_HANDLER;    //PIO0_9  Wakeup
	void* PIO0_10_WAKEUP_HANDLER;   //PIO0_10  Wakeup
	void* PIO0_11_WAKEUP_HANDLER;   //PIO0_11  Wakeup
	void* PIO1_0_WAKEUP_HANDLER;    //PIO1_0  Wakeup
	void* PIO1_1_WAKEUP_HANDLER;    //PIO1_1  Wakeup
	void* PIO1_2_WAKEUP_HANDLER;    //PIO1_2  Wakeup
	void* PIO1_3_WAKEUP_HANDLER;    //PIO1_3  Wakeup
	void* PIO1_4_WAKEUP_HANDLER;    //PIO1_4  Wakeup
	void* PIO1_5_WAKEUP_HANDLER;    //PIO1_5  Wakeup
	void* PIO1_6_WAKEUP_HANDLER;    //PIO1_6  Wakeup
	void* PIO1_7_WAKEUP_HANDLER;    //PIO1_7  Wakeup
	void* PIO1_8_WAKEUP_HANDLER;    //PIO1_8  Wakeup
	void* PIO1_9_WAKEUP_HANDLER;    //PIO1_9  Wakeup
	void* PIO1_10_WAKEUP_HANDLER;   //PIO1_10  Wakeup
	void* PIO1_11_WAKEUP_HANDLER;   //PIO1_11  Wakeup
	void* PIO2_0_WAKEUP_HANDLER;    //PIO2_0  Wakeup
	void* PIO2_1_WAKEUP_HANDLER;    //PIO2_1  Wakeup
	void* PIO2_2_WAKEUP_HANDLER;    //PIO2_2  Wakeup
	void* PIO2_3_WAKEUP_HANDLER;    //PIO2_3  Wakeup
	void* PIO2_4_WAKEUP_HANDLER;    //PIO2_4  Wakeup
	void* PIO2_5_WAKEUP_HANDLER;    //PIO2_5  Wakeup
	void* PIO2_6_WAKEUP_HANDLER;    //PIO2_6  Wakeup
	void* PIO2_7_WAKEUP_HANDLER;    //PIO2_7  Wakeup
	void* PIO2_8_WAKEUP_HANDLER;    //PIO2_8  Wakeup
	void* PIO2_9_WAKEUP_HANDLER;    //PIO2_9  Wakeup
	void* PIO2_10_WAKEUP_HANDLER;   //PIO2_10  Wakeup
	void* PIO2_11_WAKEUP_HANDLER;   //PIO2_11  Wakeup
	void* PIO3_0_WAKEUP_HANDLER;    //PIO3_0  Wakeup
	void* PIO3_1_WAKEUP_HANDLER;    //PIO3_1  Wakeup
	void* PIO3_2_WAKEUP_HANDLER;    //PIO3_2  Wakeup
	void* PIO3_3_WAKEUP_HANDLER;    //PIO3_3  Wakeup
	void* I2C_HANDLER;              //I2C
	void* TIMER16_0_HANDLER;        //16-bit Timer 0 handler
	void* TIMER16_1_HANDLER;        //16-bit Timer 1 handler
	void* TIMER32_0_HANDLER;        //32-bit Timer 0 handler
	void* TIMER32_1_HANDLER;        //32-bit Timer 1 handler
	void* SSP_HANDLER;              //SSP
	void* UART_HANDLER;             //UART
	void* USB_IRQ_HANDLER;          //USB IRQ
	void* USB_FIQ_HANDLER;          //USB FIQ
	void* ADC_HANDLER;              //ADC
	void* WDT_HANDLER;              //WDT
	void* BOD_HANDLER;              //BOD
	void* FLASH_HANDLER;            //Flash
	void* PIOINT3_HANDLER;          //PIO INT3
	void* PIOINT2_HANDLER;          //PIO INT2
	void* PIOINT1_HANDLER;          //PIO INT1
	void* PIOINT0_HANDLER;          //PIO INT0
} VECTOR_TABLE;

/* ---------------------------------
   --- System Control Block (SCB ---
   ---------------------------------
 
 Low level interrupt, exception and handler configuration, overall system control.
 This information is not taken from the LPC datasheet, but from ARM's info center.
 It is identical for most (all?) Cortex M3s. */

typedef struct SCB_STRUCT {
	HW_RO CPUID;	//CPUID base register
	HW_RW ICSR;		//Interrupt Control and State Register
	HW_RW VTOR;		//Vector Table Offset register
	HW_RW AIRCR;	//Application Interrupt and Reset Control Register
	HW_RW SCR;		//System Control Register
	HW_RW CCR;		//Configuration and Control Register
	HW_RW SHPR1;	//System Handler Priority Register 1
	HW_RW SHPR2;	//System Handler Priority Register 2
	HW_RW SHPR3;	//System Handler Priority Register 3
	HW_RW SHCRS;	//System Handler Control and State Register
	HW_RW CFSR;		//Configurable Fault Status Register
	HW_RW MMSR;		//MemManage Fault Status Register
} SCB_STRUCT;

typedef enum AIRCR_BITS {
	AIRCR_VECTKEY = (0xFA05 << 16),
	AIRCR_ENDIAN_MASK = 1 << 15,
	AIRCR_LITTLE_ENDIAN = 0,
	AIRCR_BIG_ENDIAN = 1,
	AIRCR_PRIGROUP_MASK = 0x7 << 8,
	AIRCR_PRIGROUP_BASE = 1 << 8,
	AIRCR_SYSRESETREQ = 1 << 2
} AIRCR_BITS;

#define SCB ((SCB_STRUCT*)0xe00ed00)

/** SCB_ACTLR (Auxiliary Control Register)
 is off the other block, so it's addressed separately */
#define SCB_ACTLR ((HW_RW*)0xe00e008)





#endif
