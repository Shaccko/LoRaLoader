__attribute__((naked, noreturn)) void __bootloader_reset(void) {
	extern long _sdata, _edata, _sidata, _sbss, _ebss;

	/* Memset bss 0, copy data to RAM */
	for (long* dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
	for (long* dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

	extern void boot(void);
	boot();

	for(;;);
}

extern void _estack(void);
extern void SysTick_Handler(void);
extern void EXTI3_IRQHandler(void);

__attribute__((section(".vectors"))) void (*tab[16 + 91])(void) = {
	_estack, __bootloader_reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, EXTI3_IRQHandler
};





