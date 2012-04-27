#include "anykey.h"

#define LED_PORT 0
#define LED_PIN 7

#define VGA_SYNC_PORT 1
#define VGA_HSYNC_PIN 4
#define VGA_VSYNC_PIN 5
#define VGA_RGB_PORT 2
#define VGA_R_PIN 2
#define VGA_G_PIN 1
#define VGA_B_PIN 0

uint32_t lineCounter = 0;
uint32_t frameCounter = 0;

void main(void) {
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);

	GPIO_SetDir(VGA_RGB_PORT, VGA_R_PIN, GPIO_Output);
	GPIO_SetDir(VGA_RGB_PORT, VGA_G_PIN, GPIO_Output);
	GPIO_SetDir(VGA_RGB_PORT, VGA_B_PIN, GPIO_Output);

	GPIO_SetDir(VGA_SYNC_PORT, VGA_HSYNC_PIN, GPIO_Output);
	GPIO_SetDir(VGA_SYNC_PORT, VGA_VSYNC_PIN, GPIO_Output);

	lineCounter = 0;
	frameCounter = 0;	

	SYSCON_StartSystick(2056);
	uint32_t counter = 0;
}


void systick(void) {

	__asm(
		"MOVW R2, #0x0000\n"		// R2 = GPIO1 base
		"MOVT R2, #0x5001\n"	

		"MOVW R0, #0x000\n"		// Set hsync low
		"STR R0, [R2, #0x40]\n" 	

		"MOV R3, #20\n"			// hsync wait loop
		"WAITHSYNC:\n"
		"SUBS R3, R3, #1\n"
		"BNE WAITHSYNC\n"

		"MOVW R0, #0xfff\n"		// Set hsync high
		"STR R0, [R2, #0x40]\n" 	

		"MOVT R2, #0x5002\n"		// R2 = GPIO2 base

		"MOV R3, #110\n"		// hsync back porch wait loop
		"WAITBACKPORCH:\n"
		"SUBS R3, R3, #1\n"
		"BNE WAITBACKPORCH\n"

		"MOVW R0, #0x007\n"		// Count from white to black
		
		"COLORLOOP:\n"

		"STR R0, [R2, #0x1c]\n" 	//set color
		
		"MOV R3, #64\n"		//color stripe wait loop
		"WAITCOLORSTRIPE:\n"
		"SUBS R3, R3, #1\n"
		"BNE WAITCOLORSTRIPE\n"

		"SUBS R0, R0, #1\n"		//next color
		"BNE COLORLOOP\n"

		"STR R0, [R2, #0x1c]\n" 	//set black


	: : : "r0", "r1", "r2", "r3", "cc");

	lineCounter++;
	if (lineCounter==622) {
		GPIO_WriteOutput(VGA_SYNC_PORT, VGA_VSYNC_PIN, 0);
	} else if (lineCounter>=624) {
		GPIO_WriteOutput(VGA_SYNC_PORT, VGA_VSYNC_PIN, 1);
		lineCounter = 0;
		frameCounter++;
	}

	GPIO_WriteOutput(LED_PORT, LED_PIN, (frameCounter/28)&1);
}

/* 
void scanline(void) {
// Register usage in scan line:
//	r0: current bitmap (16 pixels in char)
//	r1: bit extracted from bitamp
//	r2: GPIO1 base address (also used as zero)
//	r3: bitmap array

	__asm (
		"MOVW.W R2, #0x0000\n"
		"MOVT.W R2, #0x5001\n"
		"MOVW.W R3, #0x0000\n"
		"MOVT.W R3, #0x1000\n"

//for each char
		"LDRH R0, [R3, #0]\n"
		"STR R2,[R2, #0x38]\n"	//set to black
		"SBFX R1, R0, #15, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #14, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #13, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #12, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #11, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #10, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #9, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #8, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #7, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #6, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #5, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #4, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #3, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #2, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #1, #1\n"
		"STR R1,[R2, #0x38]\n"
		"SBFX R1, R0, #0, #1\n"
		"STR R1,[R2, #0x38]\n"
		: : : "r0", "r1", "r2", "r3", "cc"
	);
}

*/