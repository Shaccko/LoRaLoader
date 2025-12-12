#pragma once

#include <stdint.h>

#define FLASH ((struct flash*) 0x40023C00)
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

struct flash {
	volatile uint32_t ACR, KEYR, OPTKEYR, SR, CR, OPTCR;
};

static inline void unlock_flash(void) {
	FLASH->KEYR = KEY1;
	FLASH->KEYR = KEY2;
}

static inline void lock_flash(void) {
	FLASH->CR |= BIT(31);
}

void clear_flash_sectors(uint8_t sectors) {
void write_flash_b(struct ota_pkt* ota_pkt) {
