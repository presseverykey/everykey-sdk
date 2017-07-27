#include "everycdc.h"
#include "everypio.h"

int counter;

void main(void) {
	everycdc cdc;
	everycdc_init(&cdc);

	everypio_write(LED, false);

	while(true) {
		int b;
		if ( -1 !=  (b = everycdc_read_byte(&cdc)) ) {
			uint8_t ch = b & 0xff;
			++counter;
			everypio_write(LED, counter & 1);
			
			if ((ch >= 'a') && (ch <= 'z')) ch -= 'a'-'A';
			else if ((ch >= 'A') && (ch <= 'Z')) ch += 'a'-'A';
			if ((ch == 'a') || (ch == 'A')) ch = '4';
			if ((ch == 'i') || (ch == 'I')) ch = '1';
			if ((ch == 'o') || (ch == 'O')) ch = '0';
			if ((ch == 'e') || (ch == 'E')) ch = '3';
			if ((ch == 'l') || (ch == 'L')) ch = '7';
			if (ch == 's') ch = 'z';
			if (ch == 'S') ch = 'Z';

			everycdc_write_byte(&cdc, ch);
		}
	}
}

