#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

#include <packet_parser.h>

#define MAIN_FLASH (1 | 2 | 3 | 4 | 5)
#define FLASH_SWAP (6)
#define FLASH_BACKUP (7)
#define FLASH ((struct flash*) 0x40023C00)
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

#define FLASH_ADDR_VAL(x) (*(uint32_t*)x)
#define FLASH_ADDR(x) ((uint32_t*)x)

extern uint32_t _sflash, _eflash;
extern uint32_t _sflash_swap, _eflash_swap;
extern uint32_t _sflash_backup, _eflash_backup;

struct flash {
	volatile uint32_t ACR, KEYR, OPTKEYR, SR, CR, OPTCR;
};


void clear_flash_sectors(uint8_t sectors);
void write_flash(uint8_t* bin_data, uint32_t* flash_addr);
void set_flash_ptr(uint32_t* flash_addr);
void swap_ota_flash(void);

#endif
