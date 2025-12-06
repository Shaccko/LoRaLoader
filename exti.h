#ifndef __EXTI_H__
#define __EXTI_H__

#include <stdint.h>

#include <hal.h>

/* To config EXTI */
#define SYSCFG ((struct syscfg*) (0x40013800))
#define EXTI ((struct exti*) (0x40013C00))
#define NVIC ((volatile uint32_t*)0xE000E100)


struct scb {
	volatile uint32_t CPUID, ICSR, VTOR, AIRCR, SCR, CCR, SHPR1, SHPR2,
		 SHCSR;
};

struct syscfg {
	volatile uint32_t MEMRMP, PMC, EXTICR[4], CMPCR;
};

struct exti {
	volatile uint32_t IMR, EMR, RTSR, FTSR, SWIER, PR;
};

enum { FALLING, RISING };

void enable_line_interrupt(uint8_t line, uint8_t port, uint8_t trigger_type);
void EXTI0_IRQHandler(void);

#endif
