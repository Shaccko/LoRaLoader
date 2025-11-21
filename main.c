#include <stdint.h>
#include <stdio.h>

#include <uart.h>
#include <rcc.h>


int main() {
	uart2_init();
	systick_init();

	for(;;) {
		printf("We are in app!\r\n");
		delay(1000);
	}
}

