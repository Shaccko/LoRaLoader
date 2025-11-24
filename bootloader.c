#include <stdint.h>
#include <stdio.h>

#include <uart.h>
#include <hal.h>
#include <rcc.h>


static inline void blink_led(void) {
	uint32_t led_pin = PIN_NUM(5);
	uint8_t led_port = 'A';

	gpio_set_mode(led_pin, GPIO_MODE_OUTPUT, led_port);

	size_t i;
	for (i = 0; i < 10; i++) {
		gpio_write_pin(led_pin, GPIO_PIN_SET, led_port);
		delay(50);
		gpio_write_pin(led_pin, GPIO_PIN_RESET, led_port);
		delay(50);
	}
}


void boot(void) {
	extern uint32_t __app_start;

	systick_init();
	uart2_init();

	/* Bootloader stuff */
	printf("Inside bootloader!\r\n");
	uint32_t app_flash = (uint32_t)(&__app_start);
	if (((*(uint32_t*) app_flash) & 0x2FFE0000) == 0x20020000) {
		blink_led();

		/* Disable IRQ and systick for critical statements */
		SYSTICK->CTRL = 0;
		SYSTICK->LOAD = 0;
		SYSTICK->VAL = 0;

		uint32_t vt_msp = (*(uint32_t*)app_flash);
		uint32_t reset = (*(uint32_t*)(app_flash + 4));
		void (*app)(void) = ((void(*)(void)) reset);

		__asm volatile("msr msp, %0" :: "r"(vt_msp) : "memory");
		app();
	}
	else {
	}

	for(;;);
}







