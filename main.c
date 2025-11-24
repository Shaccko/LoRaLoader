#include <stdint.h>
#include <stdio.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>


int main() {
	uart2_init();
	systick_init();

	for(;;) {
		printf("yep\r\n");
		delay(500);
	}

}
