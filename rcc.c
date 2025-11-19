
#include <stdint.h>
#include <hal.h>
#include <rcc.h>

volatile uint32_t s_ticks = 0;

inline void systick_init(void) {
	uint32_t ms_tick = CLK_FREQ/1000;

	SYSTICK->LOAD = ms_tick - 1;
	SYSTICK->VAL = 0;
	SYSTICK->CTRL = BIT(0) | BIT(1) | BIT(2);
	RCC->APB2ENR |= BIT(14); /*Enable systick and SYSCFG*/
}

void SysTick_Handler(void) {
	s_ticks++;
}
