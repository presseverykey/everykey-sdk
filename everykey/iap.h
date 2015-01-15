/** In-App Programming - flash write tools. Note that using IAP requires a specific linker script.
The IAP routines use the highest 32 bytes of RAM, so it's not available for the remaining application. */

#ifndef _IAP_
#define _IAP_

#include "types.h"

#define FLASH_NUM_SECTORS 8     /* LPC1313 / LPC1343 */
#define FLASH_SECTOR_SIZE 4096
#define FLASH_NUM_PAGES 128     /* LPC1313 / LPC1343 */
#define FLASH_PAGE_SIZE 256
#define FLASH_PAGE_ADDRESS(page) ((void*)(0x00000000 + FLASH_PAGE_SIZE*(page)))
#define FLASH_SECTOR_ADDRESS(sector) ((void*)(0x00000000 + FLASH_SECTOR_SIZE*(sector)))

typedef enum {
    CMD_SUCCESS                                = 0,
    INVALID_COMMAND                            = 1,
    SRC_ADDR_ERROR                             = 2,
    DST_ADDR_ERROR                             = 3,
    SRC_ADDR_NOT_MAPPED                        = 4,
    DST_ADDR_NOT_MAPPED                        = 5,
    COUNT_ERROR                                = 6,
    INVALID_SECTOR                             = 7,
    SECTOR_NOT_BLANK                           = 8,
    SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION    = 9,
    COMPARE_ERROR                              = 10,
    BUSY                                       = 11,
    PARAM_ERROR                                = 12,
    ADDR_ERROR                                 = 13,
    ADDR_NOT_MAPPED                            = 14,
    CMD_LOCKED                                 = 15,
    INVALID_CODE                               = 16,
    INVALID_BAUD_RATE                          = 17,
    INVALID_STOP_BIT                           = 18,
    CODE_READ_PROTECTION_ENABLED               = 19
} IAP_Result;

/** Don't mix up sectors (4K) and blocks (256 bytes). Both are used. */

/** Prepares a flash sector for writing. All interrupts must be turned off during this call.
 @param sector sector to erase
 @return an IAP_Result error code */
uint8_t iap_prepare_sector(uint8_t sector);

/** Erases a flash sector. All interrupts must be turned off during this call.
 @param sector sector to erase
 @return an IAP_Result error code */
uint8_t iap_erase_sector(uint8_t sector);

/** Writes a flash page. Sector must be prepared and all interrupts must be turned off during this call.
 @param page page to write
 @param source source buffer. Must be 32-bit aligned. Must be at least FLASH_PAGE_SIZE long.
 @return an IAP_Result error code */
uint8_t iap_write_page(uint8_t page, void* source);

#endif
