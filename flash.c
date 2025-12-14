#include <stdint.h>

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
	unlock_flash();
	while (FLASH->SR & BIT(16));
	/* Set sector erase bit, indicate sectors to erase */
	FLASH->CR |= BIT(1) | (uint8_t) ((sectors << 3U)); 
	FLASH->CR |= BIT(16); /* Set start bit */
	while (FLASH->SR & BIT(16));
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

	unlock_flash();
	while (FLASH->SR & BIT(16));
	size_t i;
	uint32_t* sota_flash = (uint32_t*)&__sota_flash;
	uint32_t chunk_word = 0;
	/* Maybe transmitter could send words instead of single bytes */
	/* Write words to Flash B region */
	for (uint32_t word = 0U; i < ota_pkt->chunk_size; i = i + 4U) {
		chunk_word = ((uint32_t) ota_pkt->chunk_data[word + 0U] << 24) |
			((uint32_t) ota_pkt->chunk_data[word + 1U] << 16) |
			((uint32_t) ota_pkt->chunk_data[word + 2U] << 8) |
			((uint32_t) ota_pkt->chunk_data[word + 3U] << 0);
		*(sota_flash + word) = chunk_word;
	}
	while (FLASH->SR & BIT(16));
	lock_flash();
}
