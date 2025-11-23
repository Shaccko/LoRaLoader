#include <stdint.h>
#include <stdio.h>

#include <uart.h>
#include <hal.h>
#include <rcc.h>

#define VTOR (*(volatile uint32_t*) 0xE000ED08)

static inline void blink_led(void) {
	uint32_t led_pin = PIN_NUM(5);
	uint8_t led_port = 'A';

	gpio_set_mode(led_pin, GPIO_MODE_OUTPUT, led_port);

	size_t i;
	for (i = 0; i < 5; i++) {
		gpio_write_pin(led_pin, GPIO_PIN_SET, led_port);
		delay(500);
		gpio_write_pin(led_pin, GPIO_PIN_RESET, led_port);
		delay(500);
	}
}


void boot(void) {
	systick_init();
	uart2_init();

	/* Bootloader stuff */
	printf("Working boot\r\n");
	delay(500);
	printf("Still working\r\n");


	SYSTICK->CTRL = 0;
	SYSTICK->LOAD = 0;
	SYSTICK->VAL = 0;
	
	/* Disable IRQ and systick for critical statements */
	/* Compiler screws these registers up without volatile */
	volatile uint32_t vt_msp = (*(volatile uint32_t*)(0x08004000 + 0));
	volatile uint32_t reset =  (*(volatile uint32_t*)(0x08004000 + 4));
	VTOR = vt_msp;
	void (*app)(void) = (void(*)(void)) reset;

	__asm volatile("msr msp, %0" :: "r"(vt_msp) : "memory");
	app();

	for(;;);
}







