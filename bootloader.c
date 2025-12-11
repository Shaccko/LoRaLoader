#include <stdint.h>
#include <stdio.h>

#include <uart.h>
#include <hal.h>
#include <rcc.h>

#define FLASH ((struct flash*) 0x40023C00)
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

struct flash {
	volatile uint32_t ACR, KEYR, OPTKEYR, SR, CR, OPTCR
};


static inline void blink_led(void) {
	uint32_t led_pin = PIN_NUM(5);
	uint8_t led_port = 'A';

	gpio_set_mode(led_pin, GPIO_MODE_OUTPUT, led_port);

	size_t i;
	for (i = 0; i < 10; i++) {
		gpio_write_pin(led_port, led_pin, GPIO_PIN_SET);
		delay(50);
		gpio_write_pin(led_port, led_pin, GPIO_PIN_RESET);
		delay(50);
	}
}

static inline void unlock_flash(void) {
	struct flash* flash = FLASH;
	flash->FKEYR = KEY1;
	flash->FKEYR = KEY2;
}

static inline void lock_flash(void) {
	struct flash* flash = FLASH;
	flash->CR |= BIT(31);
}

/* Sector Erase */
/* Check BSY in SR for no flash mem op
 * Set SER bit, select sector to clean
 * Set STRT bit in CR
 * Wait for BSY
 */
static void clear_flash_sectors(uint8_t sectors) {
	struct flash* flash = FLASH;

	unlock_flash();
	while (flash->SR & BIT(16));
	/* Set sector erase bit, indicate sectors to erase */
	flash->CR |= BIT(1) | (uint8_t) ((sectors << 3U)); 
	flash->CR |= BIT(16); /* Set start bit */
	while (flash->SR & BIT(16));
	lock_flash();
}

static void write_flash_sectors(uint8_t sectors) {
	struct flash* flash = FLASH;

	unlock_flash();
	while (flash ->SR & BIT(16));

	lock_flash();
}

/* According to ARM's startup procedure, our SP and vec table 
 * gets fetched from our applications very first address, 
 * +4 that and we grab PC and _reset, knowing this we can 
 * execute some asm to place our MSP on app, and inside _reset of 
 * app, make sure we change our vec table to our application.
 */
void boot(void) {
	extern uint32_t __app_start;

	systick_init();
	uart2_init();

	/* Bootloader stuff */
	printf("Inside bootloader!\r\n");

	/* Programming flash */
	/* Check BSY 
	 * Set PG in CR
	 * Perform data write ops to desired mem addresses
	 * Wait for BSY to clear
	 */

	/* Does code exist? We check this by AND'ing MSP that could 
	 * *potentially* sit at 0x00000000 of our main app
	 */
	uint32_t app_flash = (uint32_t)(&__app_start); /* Grab address of app's start symbol from linker */
	if (((*(uint32_t*) app_flash) & 0x2FFE0000) == 0x20020000) {
		blink_led();

		/* Disable systick for critical statements */
		SYSTICK->CTRL = 0;
		SYSTICK->LOAD = 0;
		SYSTICK->VAL = 0;

		/* Grab msp (start of app's addr) 
		 * and reset (+4 from start)
		 */
		uint32_t vt_msp = (*(uint32_t*)app_flash);
		uint32_t reset = (*(uint32_t*)(app_flash + 4));
		void (*app)(void) = ((void(*)(void)) reset);

		/* Some asm to switch msp from bootloader to main app */
		__asm volatile("msr msp, %0" :: "r"(vt_msp) : "memory");

		/* Execute _reset of app */
		app();
	}
	else {
		printf("No application exists.\n");
	}

	/* Run forever */
	for(;;);
}
