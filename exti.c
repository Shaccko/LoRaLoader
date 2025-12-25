#include <stdint.h>

#include <hal.h>
#include <exti.h>
#include <rcc.h>
#include <uart.h>

static const uint8_t EXTI_IRQ[16] = {
	6, 7, 8, 9, 10, /*EXTI0-4*/
	23, 23, 23, 23, 23, /*EXTI[9:5]*/
	40, 40, 40, 40, 40, 40  /*EXTI[15:10]*/
};

void enable_line_interrupt(uint16_t pin, uint8_t port, uint8_t trigger_type) {
	struct syscfg* syscfg = SYSCFG;
	struct exti* exti = EXTI;
	uint8_t pin_num = 0;
	
	/* Enable SYSCFG clock */
	RCC->APB2ENR |= BIT(14);

	while ((pin >> pin_num) != 1U) {
		pin_num++;
	}

	/* Enable line through EXTICR */
	uint8_t shift = (uint8_t)((pin_num & 3U) * 4U);
	syscfg->EXTICR[pin_num >> 2] &= ~(0xFU << shift);
	syscfg->EXTICR[pin_num >> 2] |= (uint32_t) ((port - 'A') << shift);

	/* Set IMR flag */
	exti->IMR |= BIT(pin_num);

	/* Set trigger mode on line */
	if (trigger_type == RISING) {
		exti->RTSR |= BIT(pin_num);
		exti->FTSR &= BIT(pin_num);
	}
	else if (trigger_type == FALLING) {
		exti->FTSR |= BIT(pin_num);
		exti->RTSR &= BIT(pin_num);
	}
	/* Make ARM acknowledge the IRQ we want to enable */
	/* Gotten straight from core_cm3.h in cubeIDE */
	NVIC[EXTI_IRQ[pin_num] >> 5UL] = (uint32_t) (1U << (EXTI_IRQ[pin_num] & 0x1FUL));
}

void EXTI3_IRQHandler(void) {
	EXTI->PR = BIT(3); /* Line 3's bit PR */

	extern void sx1278_rx_irq(void);
	sx1278_rx_irq();
}
