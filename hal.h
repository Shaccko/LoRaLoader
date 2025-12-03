#ifndef HAL_H__
#define HAL_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <rcc.h>

#define CPACR ((volatile uint32_t*) 0xE000ED88) /* Address to enable FPU */

#define BIT(x) (1UL << (x))
#define GPIO(bank) ((struct gpio*) (0x40020000 + 0x400 * (bank)))
//#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PIN_NUM(pin) (1U << (pin))
#define PINBANK(pin) (pin >> 8)
#define BANK(port) ((port) - 'A')

#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
#define GPIO_MODE_AF_OD (GPIO_MODE_AF | 0x40)

/* GPIO Periph block */
#define GPIO(bank) ((struct gpio*) (0x40020000 + 0x400 * (bank)))

/* VTOR offset */
#define SCB ((struct scb*) (0xE000ED00))

/* SYS Ctrl or something like that under ARM hardware */
/* Just using it for FPU */
#define CPACR ((volatile uint32_t*) 0xE000ED88) /* Address to enable FPU */

/* To config EXTI */
#define SYSCFG ((struct syscfg*) (0x40013800))
#define EXTI ((struct exti*) (0x40013C00))
#define NVIC_ISER(x) ((volatile uint32_t*) 0xE000E100 + 0x1 * (x))

static const uint8_t EXTI_IRQ[7] = {
	6, 7, 8, 9, 10, /*EXTI0-4*/
	23, /*EXTI[9:5]*/
	40  /*EXTI[15:10]*/
};

struct scb {
	volatile uint32_t CPUID, ICSR, VTOR, AIRCR, SCR, CCR, SHPR1, SHPR2,
		 SHCSR;
};

struct syscfg {
	volatile uint32_t MEMRMP, PMC, EXTICR[4], CMPCR;
};

struct gpio {
	volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR,
		 AFR[2];
};

struct exti {
	volatile uint32_t IMR, EMR, RTSR, FTSR, SWIER, PR;
};

enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };
enum { LOW_SPEED, MED_SPEED, FAST_SPEED, HIGH_SPEED };
enum { FALLING, RISING };

static inline void gpio_set_mode(uint32_t pin, uint8_t MODE, uint8_t port) {
	struct gpio *gpio = GPIO(BANK(port));
	RCC->AHB1ENR |= BIT(BANK(port));
	uint32_t pin_pos = 0x00U;
	uint32_t bit_pos;


	while ((pin >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint32_t curr_pin = pin & bit_pos;

		if (curr_pin) {
			if (MODE & 0x40U) {
				gpio->OTYPER |= (1U << pin_pos);
			}

			gpio->MODER &= ~(3U << (pin_pos * 2U));
			gpio->MODER |= (MODE & 3U) << (pin_pos * 2U);
		}
		pin_pos++;
	}	
}

static inline void gpio_set_speed(uint32_t pin, uint8_t speed, uint8_t port) {
	struct gpio *gpio = GPIO(BANK(port));
	uint32_t pin_pos = 0x00U;
	uint32_t bit_pos;

	while ((pin >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint32_t curr_pin = pin & bit_pos;

		if (curr_pin) {
			gpio->OSPEEDR &= ~(3U << (pin_pos * 2));
			gpio->OSPEEDR |= (speed & 3U) << (pin_pos * 2);
		}
		pin_pos++;
	}
}

static inline void gpio_write_pin(uint8_t port, uint32_t pin, uint8_t val) {
	struct gpio *gpio = GPIO(BANK(port));
	uint32_t pin_pos = 0x00U;
	uint32_t bit_pos;

	while ((pin >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint32_t curr_pin = pin & bit_pos;

		if (curr_pin) {
			gpio->BSRR = (1U << pin_pos) << (val ? 0 : 16);
		}
		pin_pos++;
	}	

}

static inline uint32_t gpio_read_pin(uint8_t port, uint32_t pin) {
	struct gpio *gpio = GPIO(BANK(port));
	
	return gpio->IDR & pin ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

static inline void gpio_set_af(uint32_t pin, uint8_t af_num, uint8_t port) {
	struct gpio *gpio = GPIO(BANK(port));
	uint32_t pin_pos = 0x00U;
	uint32_t bit_pos;

	/* 00000000 00000111 
	 * & 7 = 00000111
	 * * 4 = 28 = 00011100
	 */

	while ((pin >> pin_pos) != 0x00U) {
		bit_pos = 0x01U << pin_pos;
		uint32_t curr_pin = pin & bit_pos;

		if (curr_pin) {
			gpio->AFR[pin_pos >> 3] &= ~(15UL << ((pin_pos & 7) * 4));
			gpio->AFR[pin_pos >> 3] |= ((uint32_t) af_num) << ((pin_pos & 7) * 4);
		}
		pin_pos++;
	}
}

static inline void enable_line_interrupt(uint8_t line, uint8_t port, uint8_t trigger_type) {
	struct syscfg* syscfg = SYSCFG;
	struct exti* exti = EXTI;

	/* Enable line through EXTICR */
	syscfg->EXTICR[line >> 2] &= ~(0xFU);
	syscfg->EXTICR[line >> 2] |= (uint32_t) ((port - 'A') << (line & 3));

	/* Set trigger mode on line */
	if (trigger_type == RISING) {
		exti->RTSR |= (1U << line);
	}
	else if (trigger_type == FALLING) {
		exti->FTSR |= (1U << line);
	}

	/* Make ARM acknowledge the IRQ we want to enable */
	*NVIC_ISER(line >> 5) = (uint32_t) (1U << EXTI_IRQ[line]);
}


static inline void disable_irq(void) {
	__asm volatile ("cpsid i" : : : "memory");
}

static inline void enable_irq(void) {
	__asm volatile ("cpsie i" : : : "memory");
}


#endif
