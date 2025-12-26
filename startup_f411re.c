__attribute__((naked, noreturn)) void _reset(void) {
	extern long _sbss, _ebss, _sidata, _sdata, _edata, _stext, _etext;
	extern long __image_curr;
	extern long _sflash;

	/* Set VTO to app's vec table */
	(*(volatile long*) 0xE000ED08) = (long) &_sflash;

	for (long* dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
	for (long* dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

	/* CRC Calculation */
	*(volatile long*) (0x40023800 + 0x30) |= 1L << 12L; /* Enable CRC clock */
	for (long* dst = &_stext; dst < &_etext; dst++) {
		*(volatile long*) (0x40023000) = *dst; /* Access CRC reg, assign and read for CRC on word */
		/* Access image_curr's CRC data position, sum it with memory CRC data register */
		*(volatile long*) (&__image_curr + 1) =
			*(volatile long*) (&__image_curr + 1) + *(volatile long*) (0x40023000);
	}
	
	/* Store metadata of operating image */
	*(volatile long*)&__image_curr = (long) &_sflash;

	*(volatile long*) (0x40023800 + 0x30) &= ~(1L << 12L); /* Reset CRC clock */

	/* Jump to main */
	extern int main(void);
	main();
	
	for(;;) (void)0;
}

extern void _estack(void);
extern void SysTick_Handler(void);
extern void EXTI3_IRQHandler(void);

__attribute__((section(".vectors"))) void (*tab[16 + 91])(void) = {
	_estack, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, EXTI3_IRQHandler
};


