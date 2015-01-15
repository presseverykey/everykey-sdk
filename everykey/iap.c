#include "iap.h"


typedef void (*IAP_FUNC)(uint32_t[], uint32_t[]);

#define IAP_ENTRY ((IAP_FUNC)(0x1fff1ff1))
#define IAP_CPU_CLOCK_KHZ 72000   /* Should be defined somewhere else */

typedef enum {
    IAP_PREPARE_SECTORS_FOR_WRITE              = 50,
    IAP_COPY_RAM_TO_FLASH                      = 51,
    IAP_ERASE_SECTORS                          = 52,
    IAP_BLANK_CHECK_SECTORS                    = 53,
    IAP_READ_PART_ID                           = 54,
    IAP_READ_BOOT_CODE_VERSION                 = 55,
    IAP_COMPARE                                = 56,
    IAP_REINVOKE_ISP                           = 57,
    IAP_READ_UID                               = 58
} IAP_Command_Code;

uint8_t iap_prepare_sector(uint8_t sector) {
	uint32_t args[5];
	uint32_t results[4];
	args[0] = IAP_PREPARE_SECTORS_FOR_WRITE;
	args[1] = sector;
	args[2] = sector;
	IAP_ENTRY(args, results);
	return results[0];
}

uint8_t iap_erase_sector(uint8_t sector) {
	uint32_t args[5];
	uint32_t results[4];
	args[0] = IAP_ERASE_SECTORS;
	args[1] = sector;
	args[2] = sector;
	args[3] = IAP_CPU_CLOCK_KHZ;
	IAP_ENTRY(args, results);
	return results[0];
}

uint8_t iap_write_page(uint8_t page, void* source) {
	uint32_t args[5];
	uint32_t results[4];
	args[0] = IAP_COPY_RAM_TO_FLASH;
	args[1] = (uint32_t)(FLASH_PAGE_ADDRESS(page));
	args[2] = (uint32_t)source;
	args[3] = FLASH_PAGE_SIZE;
	args[4] = IAP_CPU_CLOCK_KHZ;
	IAP_ENTRY(args, results);
	return results[0];
}

