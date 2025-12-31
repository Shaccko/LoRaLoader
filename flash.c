#include <stdint.h>
#include <stdio.h>

#include <packet_parser.h>
#include <hal.h>
#include <flash.h>

static size_t chunk_counter = 0;

static inline void unlock_flash(void) {
	FLASH->KEYR = KEY1;
	FLASH->KEYR = KEY2;
}

static inline void lock_flash(void) {
	FLASH->CR |= BIT(31);
}

static inline void open_flash_pg(void) {
	unlock_flash();
	while (FLASH->SR & BIT(16));
	FLASH->CR |= (2U << 8U); /* x32 word writes */
	FLASH->CR |= BIT(0);
}

static inline void close_flash_pg(void) {
	while (FLASH->SR & BIT(16));
	FLASH->CR &= ~(BIT(0));
	lock_flash();
}

/* Sector Erase */
/* Check BSY in SR for no flash mem op
 * Set SER bit, select sector to clean
 * Set STRT bit in CR
 * Wait for BSY
 */
void clear_flash_sectors(uint8_t sectors) {
	FLASH->CR = 0;
	unlock_flash();
	while (FLASH->SR & BIT(16));
	/* Set sector erase bit, indicate sectors to erase */
	FLASH->CR |= BIT(1) | (uint8_t) ((sectors << 3U)); 
	FLASH->CR |= BIT(16); /* Set start bit */
	while (FLASH->SR & BIT(16));
	FLASH->CR &= ~(BIT(1));
	lock_flash();
}

/* Programming flash */
/* Check BSY 
 * Set PG in CR
 * Perform data write ops to desired mem addresses
 * Wait for BSY to clear
 */
void write_flash(uint8_t* bin_data, uint32_t* flash_addr) {
	/* Unlock, enable PG, set paralellism */
	open_flash_pg();
	/* Write words to Flash B region */
	/* Chunk size needs to be a multiple of 4 */
	uint32_t* sflash = (uint32_t)flash_addr;
	for (uint32_t word = 0U; word <= CHUNK_SIZE; word = word + 4U) {
		/* Little endian */
		uint32_t chunk_word = ((uint32_t) bin_data[word + 0U] << 0) |
			((uint32_t) bin_data[word + 1U] << 8) |
			((uint32_t) bin_data[word + 2U] << 16) |
			((uint32_t) bin_data[word + 3U] << 24);
		*(sflash + chunk_counter) = chunk_word;
		chunk_counter++; 
	}
	/* Lock, disable PG, exit */
	close_flash_pg();
}

void swap_ota_flash(void) {
	clear_flash_sectors(FLASH_BACKUP);
	// create_main_backup();
	open_flash_pg();
	/* Create backup for mainflash, to be moved to swap region
	 * after end of swap */
	/* Deal with the fact that OTA firmware could be greater than
	 * backup sector size, so warn user and NOT make backup */
	for (uint32_t* src = &_sflash, *dst = &_sflash_backup;
			src < &_eflash;) {
		*dst++ = *src++;
	}

	/* Move swap region to main flash region */
	for (uint32_t* src = &_sflash_swap, *dst = &_sflash;
			src < &_eflash_swap;) {
		*dst++ = *src++;
	}
	close_flash_pg();
}



/* Unused flash_ptr set function 
void set_flash_ptr(uint32_t* flash_addr) {
	clear_flash_sectors(FLASH_MD_SECTOR);
	chunk_counter = 0;

	unlock_flash();
	while (FLASH->SR & BIT(16));
	FLASH->CR |= (2U << 8U);
	FLASH->CR |= BIT(0);

	printf("Setting flash_ptr to: %lX\r\n", (uint32_t)flash_addr);
	*((uint32_t*)(uint32_t)&_flash_ptr) = (uint32_t)flash_addr;

	while (FLASH->SR & BIT(16));
	FLASH->CR &= ~(BIT(0));
	lock_flash();
}
*/

/*
void set_flash_fallback(uint32_t* fallback_addr) {
	unlock_flash();
	while (FLASH->SR & BIT(16));
	FLASH->CR |= (2U << 8U);
	FLASH->CR |= BIT(0);

	flash_fallback = fallback_addr;

	while(FLASH->SR & BIT(16));
	FLASH->CR &= ~(BIT(0));
	lock_flash();
}
*/
