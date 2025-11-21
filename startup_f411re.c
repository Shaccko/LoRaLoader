__attribute__((naked, noreturn)) void _reset(void) {
	/* We memset bss to 0, copy all of initial data to sdata till edata */
	extern long _sbss, _ebss, _sidata, _sdata, _edata;
	for (long* dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
	for (long* dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

	extern void main(void);
	main();
	for(;;) (void)0;
}

extern void _estack(void);
extern void Systick_Handler(void);

/* Standard ARM + STM32 NVIC handlers */
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {
	_estack, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Systick_Handler
};
