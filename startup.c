__attribute__((naked, noreturn)) void _reset(void) {
	/* Memset .bss to zero, copy .data section to RAM register */

	extern long _sbss, _ebss, _sdata, _edata, _sidata;
	for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
	for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

	extern void main(void);
	main();
	for(;;) (void) 0;
}



extern void _estack(void);
extern void SysTick_Handler(void);


/* 16 standard and 91 STM32-specific handlers*/
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {
	_estack, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler};
