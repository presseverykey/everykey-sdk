#include "anykey/anykey.h"

/* Here's how to connect your VGA output to the board:

Anykey                 VGA

GPIO2.2  ---[200R]---  Red       (VGA Pin  1)
GPIO2.1  ---[200R]---  Green     (VGA Pin  2)
GPIO2.0  ---[200R]---  Blue      (VGA Pin  3)
GND      ----------+-  GND       (VGA Pin  5)
                   +-  Red GND   (VGA Pin  6)
                   +-  Green GND (VGA Pin  7)
                   +-  Blue GND  (VGA Pin  8)
                   +-  Sync GND  (VGA Pin 10)
GPIO1.7  ------------  HSync     (VGA Pin 13)
GPIO1.6  ------------  VSync     (VGA Pin 14)

R should be 200 Ohms. If you're curious why:
According to VGA spec, impedance of R, G, and B lines is 75 Ohms. High level should be 0.7 volts.
With 200 Ohms in series, total pin load is 275 Ohms. At 3.3V operating voltage, we have

I = 3.3V / 275 Ohms = 12mA.

This load is ok for output pins. According to LPC343 data sheet, section 9, note 14, up to 45mA
are allowed as long as the maximum device current rating is not exceeded.
Section 9.3, Fig. 17, shows that the pin will have approximately 2.5V at 12mA load, room temperature.
The 200/75 Ohms voltage divider (our resistor and the line) will make  

2.5V * (75 Ohms / 275 Ohms) = 0.681V.

Close enough, safe side.

If you don't have a 200 Ohms resistor at hand, use 220 Ohms. They are easier to get (E6 series).
Image will be slightly darker, but ok. If you have the choice, use metallic film resistors, 1%.
Power dissipation is less than 0.05W per resistor, so almost any type and shape will do.

HSync and VSync lines should be TTL level. Can work if the monitor's input impedance is high enough.

Cables should be as short as possible as we probably violate the 75 Ohms line impedance. In addition,
we add capacities and other dirt into high frequency signal lines, which should be avoided.

VGA female (output) connector, looking from outside:

+----------------------------+   +----------------------------+
 \    *   *   *   *   *     /     \    5   4   3   2   1     /
  \     *   *   *   *   *  /       \     10  9   8   7   6  /
   \  *   *   *   *   *   /         \ 15  14  13  12  11   /
    +--------------------+           +--------------------+

*/

#define LED_PORT 0
#define LED_PIN 7

#define VGA_SYNC_PORT 1
#define VGA_HSYNC_PIN 7
#define VGA_VSYNC_PIN 6
#define VGA_RGB_PORT 2
#define VGA_R_PIN 2
#define VGA_G_PIN 1
#define VGA_B_PIN 0

uint32_t lineCounter = 0;
uint32_t frameCounter = 0;

void main(void) {

	//set all our pins to digital output

	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_SetDir(VGA_RGB_PORT, VGA_R_PIN, GPIO_Output);
	GPIO_SetDir(VGA_RGB_PORT, VGA_G_PIN, GPIO_Output);
	GPIO_SetDir(VGA_RGB_PORT, VGA_B_PIN, GPIO_Output);
	GPIO_SetDir(VGA_SYNC_PORT, VGA_HSYNC_PIN, GPIO_Output);
	GPIO_SetDir(VGA_SYNC_PORT, VGA_VSYNC_PIN, GPIO_Output);

	//line counter is used for syncing, frame counter just for LED blinking

	lineCounter = 0;
	frameCounter = 0;	

	// setup periodic systick callbacks all 2057 cycles. Roughly hsync for 800 x 600 @ 56Hz
	// 72 MHz clock, 56Hz refresh, 625 lines total (600 visible + 25 vertical blanking)
	// 72000000 / 56 / 625 = 2057.14. Subtract by one for systick.

	SYSCON_StartSystick(2056);

	// we'll return here instead of looping forever. The boot code will loop for us,
	// but wait in WFI (wait for interrupt) mode, which allows the CPU to sleep while waiting.
}


void systick(void) {

	// We do timing critical output of pixel data in assembly. This is not necessarly
	// faster (although likely), but we can count cycles. Note that we run from
	// flash memory and we use branches. Flash access may introduce wait cycles if
	// memory access is too slow. Branches, if taken, require 2-4 cycles to complete
	// (1 for the instruction + 1-3 cycles to flush the pipeline, depending on memory
	// alignment, branch prediction etc.). In this case, we get good enough results.
	// However, if you have shifted pixels or jitter, avoid flash and branches.
	// Btw: This is a good point to get familiar with inline assmbly...

	__asm volatile (
		"MOVW R2, #0x0000\n"		// R2 = GPIO1 base
		"MOVT R2, #0x5001\n"	

		"MOVW R0, #0x000\n"		// Set hsync low
		"STR R0, [R2, #0x200]\n" 	

		"MOV R3, #28\n"			// hsync wait loop
		"WAITHSYNC:\n"
		"SUBS R3, R3, #1\n"
		"BNE WAITHSYNC\n"

		"MOVW R0, #0xfff\n"		// Set hsync high
		"STR R0, [R2, #0x200]\n" 	

		"MOVT R2, #0x5002\n"		// R2 = GPIO2 base

		"MOV R3, #45\n"			// hsync back porch wait loop
		"WAITBACKPORCH:\n"
		"SUBS R3, R3, #1\n"
		"BNE WAITBACKPORCH\n"

		"MOVW R0, #0x007\n"		// Count from white to black
		
		"COLORLOOP:\n"

		"STR R0, [R2, #0x1c]\n" 	//set color
		
		"MOV R3, #28\n"			//color stripe wait loop
		"WAITCOLORSTRIPE:\n"
		"SUBS R3, R3, #1\n"
		"BNE WAITCOLORSTRIPE\n"

		"SUBS R0, R0, #1\n"		//next color
		"BNE COLORLOOP\n"

		"STR R0, [R2, #0x1c]\n" 	//set black

	: : : "r0", "r1", "r2", "r3", "cc");	//No input, no output, clobbers some registers

	// housekeeping and Vsync stuff. Both is not as timing critical as pixel data,
	// so we can continue in C. Just make sure we finish before the scan line ends.
	// Note that assembly ended directly after settng color to black, so we have one stripe
	// time (approx. 100 Pixels = 200 clocks)

	lineCounter++;
	if (lineCounter==622) {
		GPIO_WriteOutput(VGA_SYNC_PORT, VGA_VSYNC_PIN, 0);
	} else if (lineCounter>=624) {
		GPIO_WriteOutput(VGA_SYNC_PORT, VGA_VSYNC_PIN, 1);
		lineCounter = 0;
		frameCounter++;
	}

	// blink LED with 1Hz. Because we can.

	GPIO_WriteOutput(LED_PORT, LED_PIN, (frameCounter/28)&1);
}
