#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>
#include <spi_stm32.h>
#include <exti.h>
#include <packet_parser.h>
#include <lora_stm32.h>

struct lora lora;
uint8_t rx_buf[CHUNK_SIZE + 4]; /* Header + CHUNK_SIZE */
volatile uint8_t rx_ready = 0;

__attribute__((section(".magic_ota_byte"))) volatile uint8_t magic_byte = 0;

int main() {
	uart2_init();
	spi1_init();
	systick_init();


	uint8_t status = new_lora(&lora);
	if (status) {
		uart_write_buf(uart2, "lora detected\r\n", 15);
	}
	lora_set_mode(&lora, RXCONT);

	for(;;) {
		if (rx_ready) {
			if (rx_buf[0] == OTA_MAGIC_BYTE) {
				magic_byte = OTA_MAGIC_BYTE;
				uart_write_buf(uart2, "Magic byte found, reset to bootloader for update\r\n", 48);
			}
			rx_ready = 0;
		}
		delay(500);
	}
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
