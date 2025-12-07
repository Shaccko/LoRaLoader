#ifndef RCC_H__
#define RCC_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RCC ((struct rcc *) 0x40023800)
#define SYSTICK ((struct systick *) 0xe000e010)
#define CLK_FREQ 16000000

extern volatile uint32_t s_ticks;

struct rcc {
	volatile uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR,
		 RESERVED0[2], APB1RSTR, APB2RSTR, RESERVED1[2], AHB1ENR,
		 AHB2ENR, RESERVED2[2], APB1ENR, APB2ENR, RESERVED3[2], 
		 AHB1LPENR, AHB2LPENR, RESERVED4[2], APB1LPENR, APB2LPENR,
		 RESERVED5[2], BDCR, CSR, RESERVED6[2], SSCGR, PLL12SCFGR, 
		 DCKCFGR;
};

struct systick {
	volatile uint32_t CTRL, LOAD, VAL, CALIB;
};


void SysTick_Handler(void);

static inline uint32_t get_tick(void) {
	return s_ticks;
}

static inline void delay(uint32_t time) {
	uint32_t tick_start = get_tick();

	while (get_tick() - tick_start < time);
}

void systick_init(void);



#endif
