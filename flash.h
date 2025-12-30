#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

#include <packet_parser.h>

#define FLASH_MD_SECTOR (1)
#define FLASHA_SECTOR (2 | 3 | 4 | 5)
#define FLASHB_SECTOR (6 | 7)
#define FLASH ((struct flash*) 0x40023C00)
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

// extern uint32_t* flash_fallback;

struct flash {
	volatile uint32_t ACR, KEYR, OPTKEYR, SR, CR, OPTCR;
};


void clear_flash_sectors(uint8_t sectors);
void write_flash(uint8_t* bin_data, uint32_t* flash_addr);
void set_flash_ptr(uint32_t* flash_addr);

#endif
