#include <stdint.h>
#include <stdio.h>

#include <spi_stm32.h>
#include <uart.h>
#include <hal.h>
#include <rcc.h>
#include <packet_parser.h>
#include <lora_stm32.h>

#define FLASH ((struct flash*) 0x40023C00)
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

#define MAX_CHUNK_SIZE 200

static uint8_t rx_ready = 0;
static uint8_t rx_buf[CHUNK_SIZE + 4];
struct lora lora;

struct flash {
	volatile uint32_t ACR, KEYR, OPTKEYR, SR, CR, OPTCR;
};

struct ota_pkt {
	uint8_t chunk_size;
	uint8_t chunk_data[MAX_CHUNK_SIZE];
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

/* Programming flash */
/* Check BSY 
 * Set PG in CR
 * Perform data write ops to desired mem addresses
 * Wait for BSY to clear
 */
static void write_flash_b(struct ota_pkt* ota_pkt) {
	extern uint32_t _sota_flash;
	struct flash* flash = FLASH;

	unlock_flash();
	while (flash ->SR & BIT(16));
	size_t i;
	uint32_t* sota_flash = (uint32_t*)&_sota_flash;
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
	while (flash ->SR & BIT(16));
	lock_flash();
}

static inline void ota_case_machine(void) {
	struct ota_pkt out_pkt;
	int ack_timeout = 0;
	while(1) {
		delay(1);
		if (rx_ready && (get_ota_state() == 1)) {
			uint8_t state = parse_packet_state(rx_buf, &out_pkt);
			if (state == PKT_PASS) {
				write_flash_b(&out_pkt);
			}
			lora_transmit(&lora, &state, 1);
			ack_timeout = get_tick();
			if (((int) get_tick() - ack_timeout) > 5000) {
				kill_ota_firmware();
				return;
			}
		rx_ready = 0;
		}
	}
}


/* According to ARM's startup procedure, our SP and vec table 
 * gets fetched from our applications very first address, 
 * +4 that and we grab PC and _reset, knowing this we can 
 * execute some asm to place our MSP on app, and inside _reset of 
 * app, make sure we change our vec table to our application.
 */
void boot(void) {
	extern uint32_t __app_start;
	extern uint8_t __magic_ota_byte;

	systick_init();
	uart2_init();
	spi_init();

	/* Bootloader stuff */
	printf("Inside bootloader!\r\n");

	/* Magic OTA byte exists, expect OTA firmware packets */
	if ((*(uint32_t*)&__magic_ota_byte) == 0xEF) {
		printf("Magic OTA byte detected, sending ACK\r\n");
		uint8_t tmp = ACK_CODE;
		lora_transmit(&lora, &tmp, 1);
	}


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

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
