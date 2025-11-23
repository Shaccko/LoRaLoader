#include <stdint.h>
#include <stdio.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>


int main() {
	systick_init();
	uart2_init();

	for(;;) {
		printf("l;askd;lsakd;als\r\n");
		delay(500);
	}
}
