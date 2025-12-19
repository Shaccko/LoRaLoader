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
		blink_led();
		if (rx_ready) {
			if (rx_buf[0] == OTA_MAGIC_BYTE) {
				uint8_t tmp = OTA_MAGIC_BYTE;
				lora_transmit(&lora, &tmp, 1);
			}
			if (rx_buf[0] == OTA_PACKET_BYTE) {
				printf("Received\r\n");
			}
			rx_ready = 0;
		}
	}
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
