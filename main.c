#include <stdint.h>
#include <stdio.h>

#include <uart.h>
#include <rcc.h>


int main() {
	uart2_init();
	systick_init();

	volatile uint32_t* fpu = CPACR;
	*fpu |= 0xF << 20; /* Set FPU bits */

	for(;;) {
		delay(1000);
	}
}

