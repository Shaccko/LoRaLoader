#include <stdint.h>
#include <stdio.h>

#include <packet_parser.h>
#include <hal.h>
#include <flash.h>

static inline void unlock_flash(void) {
	FLASH->KEYR = KEY1;
	FLASH->KEYR = KEY2;
}

static inline void lock_flash(void) {
	FLASH->CR |= BIT(31);
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
void write_flash_b(struct ota_pkt* ota_pkt) {
	extern uint32_t __sota_flash;

	FLASH->CR = 0;
	/* Unlock, enable PG, set paralellism */
	unlock_flash();
	while (FLASH->SR & BIT(16));
	FLASH->CR |= (2U << 8U); /* x32 word writes */
	FLASH->CR |= BIT(0); /* PG bit */
	uint32_t* sota_flash = (uint32_t*)&__sota_flash;
	/* Maybe transmitter could send words instead of single bytes */
	/* Write words to Flash B region */
	static size_t chunk_counter = 0;
	for (uint32_t word = 0U; word <= CHUNK_SIZE - 4; word = word + 4U) {
		/* Little endian */
		uint32_t chunk_word = ((uint32_t) ota_pkt->chunk_data[word + 0U] << 0) |
			((uint32_t) ota_pkt->chunk_data[word + 1U] << 8) |
			((uint32_t) ota_pkt->chunk_data[word + 2U] << 16) |
			((uint32_t) ota_pkt->chunk_data[word + 3U] << 24);
		*(sota_flash + chunk_counter) = chunk_word;
		chunk_counter++; 
	}
	/* Lock, disable PG, exit */
	while (FLASH->SR & BIT(16));
	FLASH->CR &= ~(BIT(0));
	lock_flash();
}
