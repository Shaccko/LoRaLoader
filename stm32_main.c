#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>
#include <lora_stm32.h>
#include <spi_stm32.h>
#include <exti.h>
#include <packet_parser.h>

struct lora lora;
uint8_t rx_buf[CHUNK_SIZE + 6]; /* Header + CHUNK_SIZE */
uint8_t rx_ready = 0;

int main() {
	uart2_init();
	spi1_init();
	systick_init();

	uint8_t status = new_lora(&lora);
	if (status) {
		printf("lora detected\r\n");
	}
	lora_set_mode(&lora, RXCONT);
	
	struct ota_pkt out_pkt;

	for(;;) {
		if (rx_ready) {
			uint8_t pkt_status = validate_packets_received(rx_buf, &out_pkt);
			lora_transmit(&lora, &pkt_status, 1);
			rx_ready = 0;
		}
	}
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
