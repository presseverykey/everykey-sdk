/* Demonstration of In-App Programming (writing to flash from within an application). Note that using IAP
requires a specific linker script - the IAP routines use the highest 32 bytes of RAM, so it's not available 
for the remaining application. Also, the last 4K of flash are reserved for persistent store, so they are
not available for app code.

The following example shows reprogramming flash memory. During programming, the configuration area is
set to a "this was originally programmed" string. The application will compare the configuration page
with a "this was programmed using IAP" string. If it matches, the LED will light up. If not, the LED
will remain dark and the second string will be written into flash. As a consequence, the LED will remain
dark when booting up for the first time after programming. All subsequent starts will light up the LED, even
after disconnecting power. */

#include "everykey/everykey.h"

#define LED_PORT 0
#define LED_PIN 7

#define PERSISTENCE_SECTOR 7
#define PERSISTENCE_PAGE 112

const uint8_t* template_buffer = FLASH_PAGE_ADDRESS(PERSISTENCE_PAGE);
const uint8_t magic[] = "This text was programmed using IAP.";
uint8_t __attribute__((aligned(4))) ram_buffer[FLASH_PAGE_SIZE];

void main(void) {

	EVERY_GPIO_SET_FUNCTION(LED_PORT, LED_PIN, PIO, ADMODE_DIGITAL);
	every_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);

	memset(ram_buffer, 0, FLASH_PAGE_SIZE);
	strcpy(ram_buffer, magic);

	bool programmed = !(memcmp(ram_buffer, template_buffer, FLASH_PAGE_SIZE));
	every_gpio_write(LED_PORT, LED_PIN, programmed);

	if (!programmed) {	//Not programmed: Write ram to flash.
		iap_prepare_sector(PERSISTENCE_SECTOR);
		iap_erase_sector(PERSISTENCE_SECTOR);
		iap_prepare_sector(PERSISTENCE_SECTOR);
		iap_write_page(PERSISTENCE_PAGE, ram_buffer);
	}
}

// void systick() {
// 	static uint32_t counter = 0;
// 	counter++;
// 	uint8_t bit = counter / 64 % 10;
// 	uint8_t phase = counter % 64;
// 	uint8_t duty = 0;
// 	if (bit < 8) {
// 		duty = (status & (0x80 >> bit)) ? 40 : 10;
// 	}
// 	every_gpio_write(LED_PORT, LED_PIN, phase < duty);
// }

/* store this in the config page */
__attribute__ ((section(".config")))

const uint8_t config[] = "This is the text that was written when the device was initially flashed.";

