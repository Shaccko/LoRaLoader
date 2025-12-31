__attribute__((naked, noreturn)) void _reset(void) {
	extern long _sbss, _ebss, _sidata, _sdata, _edata, _sflash;

	/* Set VTO to app's vec table */
	(*(volatile long*) 0xE000ED08) = (long) &_sflash;
	for (long* dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
	for (long* dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

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
