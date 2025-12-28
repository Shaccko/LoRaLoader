#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

#include <packet_parser.h>

#define FLASHA_SECTOR (1 | 2 | 3 | 4 | 5)
#define FLASHB_SECTOR (6 | 7)
#define FLASH ((struct flash*) 0x40023C00)
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

struct flash {
	volatile uint32_t ACR, KEYR, OPTKEYR, SR, CR, OPTCR;
};


void clear_flash_sectors(uint8_t sectors);
void write_flash(uint8_t* bin_data, uint32_t* flash_addr);

#endif
