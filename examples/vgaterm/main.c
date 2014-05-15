#include "everykey/everykey.h"

#include "fontTex.h"

#define LED_PORT 0
#define LED_PIN 7

#define KEY_PORT 1
#define KEY_PIN 4

#define VGA_SYNC_PORT 1
#define VGA_HSYNC_PIN 7
#define VGA_VSYNC_PIN 6

#define VGA_RGB_PORT 2
#define VGA_R_PIN 2
#define VGA_G_PIN 1
#define VGA_B_PIN 0

/* Note that these are also defined as ASM defines */
#define ROWS 15
#define COLUMNS 30
//hard-coded: COLUMNS_PER_BITMAP = 16

uint8_t charMap[COLUMNS*ROWS];
uint16_t currentScanline;			
uint32_t currentFrame;			
uint16_t scanlinePixels[COLUMNS];
int16_t vOffset;

const uint8_t screen1[] = "\n\n\nPress any key to continue\n\n\n";

const uint8_t screen2[] = "\n\n\nThanks for pressing any key.\n\nPress any key to restart ";


#define SHORT_PRESS_FRAMES 2
#define LONG_PRESS_FRAMES 50

void clearScreen() {
	uint32_t i;
	for (i = 0; i<(ROWS*COLUMNS); i++) {
		charMap[i] = 0;
	}
}

void setScreen(const uint8_t* string) {
	clearScreen();
	int off = 0;
	int row = 0;
	int col = 1;
	uint8_t ch;
	while (ch = string[off++]) {
		switch (ch) {		
			case '\n':
				row = (row+1) % ROWS;
				col = 1;
				break;
			default:
				charMap[row*COLUMNS+col] = ch;
				col++;
				if (col>=COLUMNS) {
					col = 1;
					row = (row+1) % ROWS;
				}
				break;
		}
	}
}
		
void main(void) {

	const uint8_t* screens[] = {
		screen1,
		screen2
	};
	uint16_t screenCount = 2;
	
	//init pins
	every_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	every_gpio_write(LED_PORT, LED_PIN, 0);

	every_gpio_set_dir(VGA_RGB_PORT, VGA_R_PIN, OUTPUT);
	every_gpio_set_dir(VGA_RGB_PORT, VGA_G_PIN, OUTPUT);
	every_gpio_set_dir(VGA_RGB_PORT, VGA_B_PIN, OUTPUT);

	every_gpio_set_dir(VGA_SYNC_PORT, VGA_HSYNC_PIN, OUTPUT);
	every_gpio_set_dir(VGA_SYNC_PORT, VGA_VSYNC_PIN, OUTPUT);

	every_gpio_set_dir(KEY_PORT, KEY_PIN, INPUT);
	EVERY_GPIO_SET_PULL(KEY_PORT, KEY_PIN, PULL_UP);

	setScreen(screen1);

	currentScanline = 0;
	vOffset = 0;
	uint32_t currentScreen = 0;
	bool oldState = 1;
	uint32_t downFrame = 0;
	setScreen(screens[currentScreen]);

	//start video generation (one systick per scanline)
	SYSCON_StartSystick(2056);

	while (1) {
		bool newState = every_gpio_read(KEY_PORT, KEY_PIN);
		if (!newState && oldState) {
			downFrame = currentFrame;
		}
		if (newState && !oldState) {
			uint32_t downDuration = currentFrame - downFrame;
			if (downDuration > LONG_PRESS_FRAMES) {
				currentScreen = (currentScreen + screenCount - 1) % screenCount;
			} else if (downDuration > SHORT_PRESS_FRAMES) {
				currentScreen = (currentScreen + 1) % screenCount;
			}
			setScreen(screens[currentScreen]);
		}
		oldState = newState;

		int val = currentFrame % 100 - 50;
		val = val * val / 10;
		vOffset = -val;
		every_gpio_write(LED_PORT, LED_PIN, currentScreen & 1);
	}
}



void systick(void) {
	asm volatile (
		".equ GPIO1, 0x50010000\n"
		".equ GPIO2, 0x50020000\n"
		".equ LED, 0x50000200\n"
		".equ ROWS, 15\n"
		".equ COLUMNS, 30\n"
		".equ OUTPUT_LINES, 600\n"
		".equ LINES_PER_BITMAP, 32\n"
		"LDR r0, =GPIO1\n"		//1-2     r0 = GPIO1
		"MOV r1, #0\n"			//1       r1 = 0
		"STR r1,[r0, #0x200]\n"		//1       set hsync
		"LDR r2, =currentScanline\n"	//2       r2 = &scanline
		"LDRH r3, [r2]\n"		//1       r3 = scanline (last line)
		"ADD r3, r3, #1\n"		//1       r3 = scanline+1 (this line)
		"STRH r3, [r2]\n"		//2       write back line
		"LDR r1, =OUTPUT_LINES\n"	//2       r1 = 600
		"CMP r3, r1\n"			//1       Scanline in visible region?
		"BGE BLANKINGLINE\n"		//1-4     Go to blanking line if invisible

		"PIXELLINE:\n"
		//hsync is low. Wait a bit before pulling up again. This time could be used for other calculations
		"MOV r1, #28\n"			//1       r1 = wait counter 
		"PIXELLINEWAITSYNC:\n"
		"SUBS r1, r1, #1\n"		//1	  count down counter
		"BNE PIXELLINEWAITSYNC\n"    	//1-4(2)  wait loop branch
		"MOV r1, #0xff\n"		//1       r1 = 255
		"STR r1,[r0, #0x200]\n"		//1       set hsync high

		//offset pixel line
		"LDR r1, =vOffset\n"		//2       r2 = &vOffset
		"LDRH r1, [r1]\n"		//2       r1 = vOffset
		"SXTH r1, r1\n"			//1       signed extend r1
		"ADD r3, r3, r1\n"		//1       r3 = scanline + vOffset
		//calculate text line and scan line within text line
		"LSR r1, r3, #5\n"		//1	  r1 = r3 / 32 (text row number)
		"LDR r2, =ROWS\n"		//2	  r2 = row count
		"CMP r1, r2\n"			//1       current row within limits?
		"BGE LINEEND\n"			//1-4     No: bail
		"LSL r2, r1, #5\n"		//1	  r2 = r1 * 32 (first line of text row)
		"SUB r2, r3, r2\n"		//1       r2 = r3 - r2 (pixel line within text line)
		"LDR r3, =LINES_PER_BITMAP\n"	//2	  r3 = actual pixel lines per text row
		"CMP r2, r3\n"			//1       scan line in row contains actual pixels?
                "BGE LINEEND\n"			//1-4     No pixels -> bail
						//        Current register usage: r1 -> text row, r2 -> scanline in currently displayed row
		//calculate text line and bitmap line bases
		"LDR r3, =COLUMNS\n"		//1       Load column count to r3
		"MUL r1, r1, r3\n"		//1       r1 is now line offset in charMap
		"LDR r0, =charMap\n"		//2       r0 = charMap base address
		"ADD r0, r0, r1\n"		//1       r0 = text row base address
		"LDR r1, =fontTex\n"		//2       r1 = textures base address
		"ADD r1, r1, r2, lsl #8\n"	//1       r1 = line base address (base + 256*line number)
		"LDR r2, =scanlinePixels\n"	//2       r2 = scanline buffer base address
						//        Current register usage: r0 -> text row base, r1 -> bitmap current line base, r2 -> bitmap buffer base

		//blit bitmap into scanline buffer - unrolled loop
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address
		"LDRB r3, [r0], #1\n"		//2       r3 = current character, increase r0 by one
                "LDRH r3, [r1, r3, lsl #1]\n"	//2       r3 = current 16 bitmap pixels
		"STRH r3, [r2], #2\n"		//1       Store pixels to scanline, add scanline address

		//prepare for outputting pixels
		"LDR r0, =GPIO2\n"		//2       r0 = GPIO2 base
		"LDR r2, =scanlinePixels\n"	//2       r2 = scanline buffer base address

		//output pixels - unrolled loop
		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel

		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
/*
		"LDRH r3, [r2], #2\n"		//2	  load next block of bitmap
		"SBFX r1, r3, #15, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #14, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #13, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #12, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #11, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #10, #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #9,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #8,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #7,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #6,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #5,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #4,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #3,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #2,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #1,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
		"SBFX r1, r3, #0,  #1\n"	//1       extract and extend one pixel to r1
		"STR r1, [r0, #0x1c]\n"		//1       write pixel
*/

		"MOV r1, #0\n"			//1       r1 = 0
		"STR r1, [r0, #0x1c]\n"		//1       set color to black
		"B LINEEND\n"			//2-4

		"BLANKINGLINE:\n"
		//hsync is down. Wait a bit before pulling it up again.
		"MOV r1, #28\n"			//1       r1 = wait counter 
		"BLANKINGLINEWAITSYNC:\n"
		"SUBS r1, r1, #1\n"		//1	  count down counter
		"BNE BLANKINGLINEWAITSYNC\n"    //1-4(2)  wait loop branch
		"MOV r1, #0xff\n"		//1       r1 = 255
		"STR r1,[r0, #0x200]\n"		//1       set hsync high

		//dispatch based on line number (vsync low, vsync high, reset line counter, nothing)
		"LDR r1, =601\n"		//2       r1 = 601 (vsync down line)
		"CMP r3,r1\n"			//1       vsync down line?
		"BEQ VSYNCDOWN\n"		//1-4     branch to VSYNCDOWN if yes
		"LDR r1, =603\n"		//2       r1 = 603 (vsync up line)
		"CMP r3,r1\n"			//1       vsync up line?
		"BEQ VSYNCUP\n"			//1-4     branch to VSYNCUP if yes
		"LDR r1, =625\n"		//2       r1 = 625 (reset line counter)
		"CMP r3,r1\n"			//1       reset line?
		"BEQ RESETLINE\n"		//1-4     branch to RESETLINE if yes
		"B LINEEND\n"			//2-4     goto end

		"VSYNCDOWN:\n"
		"MOV r1, #0\n"			//1       r1 = 0
		"STR r1, [r0, #0x100]\n"	//2	  set vsync
		"B LINEEND\n"			//2-4     goto end

		"VSYNCUP:\n"
		"MOV r1, #0xff\n"		//1       r1 = 255
		"STR r1, [r0, #0x100]\n"	//2	  set vsync
		"B LINEEND\n"			//2-4     goto end

		"RESETLINE:\n"
		"MOV r3, #0\n"			//1       r3 = 0
		"STRH r3, [r2, #0]\n"		//2       write back line
		"LDR r1, =currentFrame\n"	//2	  r1 = &currentFrame
		"LDR r0, [r1]\n"		//2       r0 = currentFrame
		"MOV r2, #1\n"			//1       r2 = 1
		"ADD r0, r0, r2\n"		//1       r0 = currentFrame + 1
                "STR r0, [r1]\n"		//1       write currentFrame
		"LINEEND:\n"

		: 
		:
		: "r0","r1","r2","r3","cc"
	);
}
