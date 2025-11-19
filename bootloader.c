#include <stdint.h>
#include <memory.map.h>
#include <uart.h>


__attribute__((naked, noreturn)) static void start_app(uint32_t pc, uint32_t sp) {
	__asm(" msr msp, r1 /* load r1 into MSP */\n\
		bx r0	    /* branch to address at r0 */\n\
		");
}

int main(void) {
	uart2_init();
	printf("Bootloader!\r\n");
	/*uart2_deinit(); should be deinit, but not implemented yet */

	uint32_t *app_code = __aprom_start__;
	uint32_t app_sp = app_code[0];
	uint32_t app_start = app_code[1];

	start_app(app_start, app_sp);

	for(;;);
}
