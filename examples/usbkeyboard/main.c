#include "keyboard.h"


uint8_t inBuffer[8];
uint8_t outBuffer[8];

#define LED_PORT 0
#define LED_PIN 7
#define KEY_PORT 1
#define KEY_PIN 4
#define TYPING_DIVIDER 11

/** remember when the button was down */
bool buttonWasDown;

/** a timer counter for producing a sequence of keystrokes */
int32_t counter;

/** key code that is currently down */
uint8_t downKey;

/** USB HID keyboard codes. See HUT1_12.pdf (from usb.org). 255 indicates end of sequence */
const uint8_t typeSequence[] = { 23,0, 11,0, 4,0, 17,0, 14,0, 44,0, 29,0, 18,0, 24,0, 44,0,
	9,0, 18,0, 21,0, 44,0, 19,0, 21,0, 8,0, 22,0, 22,0, 12,0, 17,0, 10,0, 44,0,
	4,0, 17,0, 29,0, 44,0, 14,0, 8,0, 29,0, 40,0, 255 };

/** generate an IN report. We only use one key, no modifiers */
uint16_t inReportHandler(uint8_t reportType, uint8_t reportId) {
	inBuffer[0] = 0;
	inBuffer[1] = 0;
	inBuffer[2] = downKey;
	inBuffer[3] = 0;
	inBuffer[4] = 0;
	inBuffer[5] = 0;
	inBuffer[6] = 0;
	inBuffer[7] = 0;
	return 8;
}

/** parse an OUT report. We just read the caps lock bit and turn the LED on and off */
void outReportHandler(uint8_t reportType, uint8_t reportId, uint16_t len) {
	bool ledOn = outBuffer[0] & 0x02;	//caps lock LED
	GPIO_WriteOutput(LED_PORT, LED_PIN, ledOn);
}

void main() {
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, false);
	GPIO_SetDir(KEY_PORT, KEY_PIN, GPIO_Input);
	GPIO_SETPULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);

	buttonWasDown = false;
	downKey = 0;
	counter = -1;
	
	KeyboardInit(inBuffer, outBuffer, inReportHandler, outReportHandler);
	USB_SoftConnect();
	SYSCON_StartSystick(71999);  // 1KHz
}

void systick() {
	bool buttonDown = !GPIO_ReadInput(KEY_PORT, KEY_PIN);
	if (buttonDown && !buttonWasDown) {
		counter = 0;
	}
	buttonWasDown = buttonDown;
	
	if (counter >= 0) {
		counter++;
		if ((counter % TYPING_DIVIDER) == 1) {
			uint16_t index = counter / TYPING_DIVIDER;
			downKey = typeSequence[index];
			if (downKey == 255) {
				downKey = 0;
				counter = -1;
			}
			HIDPushReport(HID_REPORTTYPE_INPUT,0);
		}
	}
}
