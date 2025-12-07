#include <stdint.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>

int main() {
	uart2_init();
	systick_init();

	for(;;) {
		uart_write_buf(uart2, "New Firmware!", 13);
		delay(500);
	}

}
