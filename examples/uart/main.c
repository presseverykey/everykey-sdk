#include "anykey/anykey.h"
#include "anycdc.h"

#define LED_PORT 0
#define LED_PIN 7



anycdc cdc;

void main(void) {	
	anycdc_init(&cdc);

	any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	any_gpio_write(LED_PORT, LED_PIN, false);
	UART_Init(9600, 8, UART_PARITY_NONE, 1, false, NULL);

	while (1) {
		uint8_t ch;
		int b = anycdc_read_byte(&cdc);
		if (b > 0) {
			while (!UART_Write(&ch,1)) {};
			if (ch == 0x0d) {	//convert CR to CRLF
				ch = 0x0a;
				while (!UART_Write(&ch,1)) {};
			}
			any_gpio_write(LED_PORT, LED_PIN, !any_gpio_read(LED_PORT,LED_PIN));
		}

		if (UART_Read(&ch, 1)) {
			while (!anycdc_write_byte(&cdc, ch)) {};
			any_gpio_write(LED_PORT, LED_PIN, !any_gpio_read(LED_PORT,LED_PIN));
		}
	}
}
