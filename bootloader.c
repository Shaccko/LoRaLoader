#include <stdint.h>
#include <hal.h>
#include <rcc.h>

#define VTOR ((volatile uint32_t*) 0xE000ED08)

__attribute__((naked, noreturn)) static void start_app(uint32_t pc, uint32_t sp) {
	__asm volatile (
			"msr msp, r1\n"
			"bx r0\n"
		       );
}

static void init_data(void) {
	extern uint32_t _sdata, _edata, _sidata, _sbss, _ebss;
	for (volatile uint32_t* dst = &_sbss; dst < &_ebss; dst++); *dst = 0;
	for (volatile uint32_t* dst = &_sdata, volatile uint32_t* src = &_sidata; dst < &_edata) *dst++ = *src++;
}

static inline void blink_led(void) {
	uint32_t led_pin = PIN_NUM(5);
	uint8_t led_port = 'A';

	gpio_set_mode(led_pin, GPIO_MODE_OUTPUT, led_port);

	size_t i;
	for (i = 0; i < 5; i++) {
		gpio_write_pin(led_port, led_pin, GPIO_PIN_SET);
		delay(500);
		gpio_write_pin(led_port, led_pin, GPIO_PIN_RESET);
		delay(500);
	}
}

extern void _estack(void);
extern void Systick_Handler(void);

__attribute__((section(".vectors"))) void (*tab[16 + 91])(void) = {
	_estack, _bootloader, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler};
}

int _bootloader() {
	extern uint32_t _startapp, _startapp_size;


	init_data();
	systick_init();

	/* Bootloader stuff */
	blink_led();

	/* Disable IRQ for critical statements */
	disable_irq();
	uint32_t* app_sp = (uint32_t*)_startapp;
	uint32_t* app_pc = (uint32_t*)(_startapp + 4);

	/* Change our vector table from bootloader to app */
	uint32_t *curr_vtable = (uint32_t*) &_startapp;
	volatile uint32_t* vtor = VTOR;
	vtor = (uint32_t*) curr_table;

	enable_irq();
	start_app(app_pc, app_sp); /* move bootloader sp to msp, jump to main app */

	for(;;);
}











