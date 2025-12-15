#include <stdint.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>

int main() {
	uart2_init();
	systick_init();

	for(;;) {
		uart_write_buf(uart2, "New Firmware!\r\n", 15);
		delay(500);
	}

}
