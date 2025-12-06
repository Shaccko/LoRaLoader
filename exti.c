#include <stdint.h>

#include <hal.h>
#include <exti.h>
#include <rcc.h>
#include <uart.h>

static const uint8_t EXTI_IRQ[7] = {
	6, 7, 8, 9, 10, /*EXTI0-4*/
	23, /*EXTI[9:5]*/
	40  /*EXTI[15:10]*/
};

void enable_line_interrupt(uint8_t line, uint8_t port, uint8_t trigger_type) {

	struct syscfg* syscfg = SYSCFG;
	struct exti* exti = EXTI;
	
	/* Enable SYSCFG clock */
	RCC->APB2ENR |= BIT(14);

	/* Enable line through EXTICR */
	syscfg->EXTICR[line >> 2] &= ~(0xFU);
	syscfg->EXTICR[line >> 2] |= (uint32_t) ((port - 'A') << ((line & 3U) * 4));

	/* Set IMR flag */
	exti->IMR |= BIT(line);

	/* Set trigger mode on line */
	if (trigger_type == RISING) {
		exti->RTSR |= (1U << line);
	}
	else if (trigger_type == FALLING) {
		exti->FTSR |= (1U << line);
	}
	printf("Hung\r\n");

	/* Make ARM acknowledge the IRQ we want to enable */
	NVIC[EXTI_IRQ[line] >> 5UL] = (uint32_t) (1U << EXTI_IRQ[line] & 0x1FUL);
}



void EXTI0_IRQHandler(void) {
	EXTI->PR = BIT(0); /* Line 0's bit PR */

	printf("Here");

	extern void lora_rx_irq(void);
	lora_rx_irq();
}
