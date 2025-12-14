#include <stdint.h>
#include <stdio.h>

#include <spi_stm32.h>
#include <uart.h>
#include <hal.h>
#include <rcc.h>
#include <packet_parser.h>
#include <lora_stm32.h>
#include <exti.h>

#define MAX_CHUNK_SIZE 200

uint8_t rx_ready = 0;
uint8_t rx_buf[CHUNK_SIZE + 4];
struct lora lora;


static inline void blink_led_a(void) {
	uint32_t led_pin = PIN_NUM(5);
	uint8_t led_port = 'A';

	gpio_set_mode(led_pin, GPIO_MODE_OUTPUT, led_port);

	size_t i;
	for (i = 0; i < 10; i++) {
		gpio_write_pin(led_port, led_pin, GPIO_PIN_RESET);
		delay(50);
	}
}

static inline void blink_led_b(void) {
	uint32_t led_pin = PIN_NUM(5);
	uint8_t led_port = 'A';

	gpio_set_mode(led_pin, GPIO_MODE_OUTPUT, led_port);

	size_t i;
	for (i = 0; i < 5; i++) {
		gpio_write_pin(led_port, led_pin, GPIO_PIN_RESET);
		delay(500);
	}
}
static void download_ota_packets(void) {
	int ack_timeout = 0;
	while(1) {
		delay(1); /* Why is this necessary */
		if (rx_ready && (get_ota_state() == 1)) {
			printf("Received packet\r\n");
			uint8_t state = parse_packet_state(rx_buf);
			lora_transmit(&lora, &state, 1);
			ack_timeout = (int) get_tick();
			if (state == PKT_COMPLETE) return;
		rx_ready = 0;
		}
		if (((int) get_tick() - ack_timeout) > 5000) {
			kill_ota_firmware();
			return;
		}
	}
}

static void boot_flash_a(void) {
	extern uint32_t __app_start;

	/* Does code exist? We check this by AND'ing MSP that could 
	 * *potentially* sit at 0x00000000 of our main app
	 */
	uint32_t app_flash = (uint32_t)(&__app_start); /* Grab address of app's start symbol from linker */
	if (((*(uint32_t*) app_flash) & 0x2FFE0000) == 0x20020000) {
		blink_led_a();

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
		printf("Flash A does not exist.\n");
	}

	/* Run forever */
	for(;;);
}


static void boot_flash_b(void) {
	extern uint32_t __sota_flash;

	uint32_t flash_b = (uint32_t)&__sota_flash;
	/* Does code exist? We check this by AND'ing MSP that could 
	 * *potentially* sit at 0x00000000 of our main app
	 */
	if (((*(uint32_t*) flash_b) & 0x2FFE0000) == 0x20020000) {
		blink_led_b();

		/* Disable systick for critical statements */
		SYSTICK->CTRL = 0;
		SYSTICK->LOAD = 0;
		SYSTICK->VAL = 0;

		/* Grab msp (start of app's addr) 
		 * and reset (+4 from start)
		 */
		uint32_t vt_msp = (*(uint32_t*)flash_b);
		uint32_t reset = (*(uint32_t*)(flash_b + 4));
		void (*app)(void) = ((void(*)(void)) reset);

		/* Some asm to switch msp from bootloader to main app */
		__asm volatile("msr msp, %0" :: "r"(vt_msp) : "memory");

		/* Execute _reset of app */
		app();
	}
	else {
		printf("Flash B does not exist.\r\n");
	}

	for(;;);
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

	uart2_init();
	spi1_init();
	systick_init();

	new_lora(&lora);

	/*--------- Bootloader stuff ----------*/
	printf("Inside bootloader!\r\n");
	lora_set_mode(&lora, RXCONT);

	/* Magic OTA byte exists, expect OTA firmware packets */
	uint8_t magic_byte = ((*(uint8_t*)&__magic_ota_byte));
	printf("magic_byte: %x\r\n", magic_byte);
	if (magic_byte == 0xCC) {
		printf("Magic OTA byte detected, sending ACK\r\n");
		uint8_t tmp = ACK_CODE;
		lora_transmit(&lora, &tmp, 1);
		kill_ota_firmware();
		set_ota_state();
		download_ota_packets();
		/* Done, reset magic byte */
		((*(uint8_t*)&__magic_ota_byte)) = 0;
		boot_flash_b();
	}
	else {
		printf("No magic byte for flash b detected, booting from flash a\r\n");
		lora_set_mode(&lora, SLEEP);
		boot_flash_a();
	}

	/* Run forever */
	for(;;);
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
