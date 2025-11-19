#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <rcc.h>
#include <hal.h>
#include <spi.h>
#include <uart.h>
#include <LoRa.h>

#define LED_PIN(pin) (BIT(pin))
#define LED_PORT 'A'

int main(void) {
	/* Configure hal.h to hold pin bank and pins, 32bits can fit both */

	char buf[32];
	struct lora lora;
	uint8_t lora_version = 0;

	uart2_init();
	spi1_init();

	systick_init();
	lora_version = new_lora(&lora);
	if (lora_version) uart_write_buf(uart2, "lora pass\r\n", 11);
	for (;;) {
		lora_transmit(&lora, (uint8_t*)"Working", 7);
		delay(50);
	}
	return 0;
}
