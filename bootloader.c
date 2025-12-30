#include <stdint.h>
#include <stdio.h>

#include <spi_stm32.h>
#include <uart.h>
#include <hal.h>
#include <rcc.h>

#include <packet_parser.h>
#include <flash.h>


static void blink_led(void) {
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

static void jump_to_flash(uint32_t* app_flash) {
	blink_led();
	/* Disable systick for critical statements */
	disable_irq();
	SYSTICK->CTRL = 0;
	SYSTICK->LOAD = 0;
	SYSTICK->VAL = 0;

	/* Grab msp (start of app's addr) 
	 * and reset (+4 from start)
	 */

	uint32_t vt_msp = *((uint32_t*)((uint32_t)app_flash));
	uint32_t reset = *((uint32_t*)((uint32_t)app_flash + 4));
	void (*app)(void) = ((void(*)(void)) reset);

	/* Some asm to switch msp from bootloader to main app */
	__asm volatile("msr msp, %0" :: "r"(vt_msp) : "memory");

	/* Execute _reset of app */
	enable_irq();
	app();
}

/* According to ARM's startup procedure, our SP and vec table 
 * gets fetched from our applications very first address, 
 * +4 that and we grab PC and _reset, knowing this we can 
 * execute some asm to place our MSP on app, and inside _reset of 
 * app, make sure we change our vec table to our application.
 */
void boot(void) {
	extern uint32_t _sflash_a, _sflash_b;
	extern uint32_t _flash_ptr;

	uart2_init();
	systick_init();

	/*--------- Bootloader stuff ----------*/
	uart_write_buf(uart2, "Inside bootloader\r\n", 19);

	/* Does code exist? We check this by AND'ing MSP that could 
	 * *potentially* sit at 0x20020000 of our flash region 
	 */
	printf("flash_ptr: %lX\r\n", *((uint32_t*)&_flash_ptr));
	if ((*((uint32_t*)&_flash_ptr) & 0xFFFF3FFFU) == 0x08000000) {
		/* Jump to whatever flash flash_ptr is pointing at */
		/* Dereferenced address of symbol to get flash region,
		 * casted back to ptr type */
		jump_to_flash((uint32_t*)(*((uint32_t*)&_flash_ptr)));
	}
	else {
		/* Manually check which flash region is available */
		if ((*((uint32_t*)(uint32_t)&_sflash_a) & 0x2FFE0000) == 0x20020000) {
			printf("Flash a set as ptr %lX\r\n", (uint32_t)&_sflash_a);
			set_flash_ptr(&_sflash_a);
			jump_to_flash(&_sflash_a);
		}

		if ((*((uint32_t*)(uint32_t)&_sflash_b) & 0x2FFE0000) == 0x20020000) {
			printf("Flash b set as ptr %lX\r\n", (uint32_t)&_sflash_b);
			set_flash_ptr(&_sflash_b);
			jump_to_flash(&_sflash_b);
		}
	}


	/* Run forever */
	for(;;) (void)0;
}


/*
 * __attribute__((section(".flash_ptr"))) static uint8_t* flash_ptr = &_sflash_a;
 */


/*
 * if (flash_a exists) {
 * 	flash_ptr = flash_a;
 * }
 * elif (flash_b exists) {
 * 	flash_ptr = flash_b;
 * }
 * else
 * 	set flash pointer
 * 	flash_ptr
 *}
 *
 *
 *
 *if (flash_ptr) {
 	boot to flash_ptr
else {
	check which region has valid flash
	manually boot to that region
}
when flash is done downloading, set flash pointer to downloaded region
if region's crc check fails, fall back to safe flash region
else set safe flash region to flash pointer

safe flash region should be set somehow somewhere
 *
 * Write flash pointer into a special flash region when done processing last packet
 * Write flash CRC into special crc region, if no crc is written, inform user that flash isn't verified and may be prone to errors, but boot anyways
 *
 *
 */
