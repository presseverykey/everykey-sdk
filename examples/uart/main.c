#include "everykey/everykey.h"
#include "everycdc.h"

#define LED_PORT 0
#define LED_PIN 7



everycdc cdc;

void main(void) {	
	everycdc_init(&cdc);

	every_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	every_gpio_write(LED_PORT, LED_PIN, false);
	UART_Init(9600, 8, UART_PARITY_NONE, 1, false, NULL);

	while (1) {
		uint8_t ch;
		int b = everycdc_read_byte(&cdc);
		if (b > 0) {
			ch = b;
			while (!UART_Write(&ch,1)) {};
			if (ch == 0x0d) {	//convert CR to CRLF
				ch = 0x0a;
				while (!UART_Write(&ch,1)) {};
			}
			every_gpio_write(LED_PORT, LED_PIN, !every_gpio_read(LED_PORT,LED_PIN));
		}

		if (UART_Read(&ch, 1)) {
			while (!everycdc_write_byte(&cdc, ch)) {};
			every_gpio_write(LED_PORT, LED_PIN, !every_gpio_read(LED_PORT,LED_PIN));
		}
	}
}
