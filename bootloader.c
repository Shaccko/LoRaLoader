#include <stdint.h>
#include <stdio.h>

#include <spi_stm32.h>
#include <uart.h>
#include <hal.h>
#include <rcc.h>
#include <packet_parser.h>

#define MAX_CHUNK_SIZE 200

volatile uint8_t rx_ready = 0;
uint8_t rx_buf[CHUNK_SIZE + 1];

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

/*
static void download_ota_packets(void) {
	int ack_timeout = 0;
	while(1) {
		delay(1); 
		if (rx_ready && (get_ota_state() == 1)) {
			uint8_t state = parse_packet_state(rx_buf);
			ack_timeout = (int) get_stm32_tick();
			if (state == PKT_COMPLETE) return;
		rx_ready = 0;
		}
		if (((int) get_stm32_tick() - ack_timeout) > 5000) {
			kill_ota_firmware();
			return;
		}
	}
}
*/

/* According to ARM's startup procedure, our SP and vec table 
 * gets fetched from our applications very first address, 
 * +4 that and we grab PC and _reset, knowing this we can 
 * execute some asm to place our MSP on app, and inside _reset of 
 * app, make sure we change our vec table to our application.
 */
void boot(void) {
	extern uint32_t __app_start;

	uart2_init();
	systick_init();

	/*--------- Bootloader stuff ----------*/
	uart_write_buf(uart2, "Inside bootloader\r\n", 19);

	/* Does code exist? We check this by AND'ing MSP that could 
	 * *potentially* sit at 0x20020000 of our flash region 
	 */
	uint32_t app_flash = (uint32_t)(&__app_start); /* Grab address of app's start symbol from linker */
	if (((*(uint32_t*) app_flash) & 0x2FFE0000) == 0x20020000) {

		blink_led();
		/* Disable systick for critical statements */
		disable_irq();
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
		enable_irq();
		app();
	}

	/* Run forever */
	for(;;) (void)0;
}

void sx1278_rx_irq(void) {
	rx_ready = 1;
}
