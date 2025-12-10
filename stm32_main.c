#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
uint8_t rx_ready = 0, ota_transfer_rdy = 0;

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
		delay(1);
		if (rx_ready) {
			if (ota_transfer_rdy && rx_buf[0] == OTA_BYTE) {
				printf("Received packet. \r\n");
				uint8_t pkt_status = validate_packets_received(rx_buf, &out_pkt);
				lora_transmit(&lora, &pkt_status, 1);

				if (pkt_status == PKT_COMPLETE) {
					printf("All packets received.\r\n");
					ota_transfer_rdy = 0;
				}
			}
		}
		rx_ready = 0;
	}
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	if (rx_buf[0] == OTA_TX_START) {
		uint8_t tmp = ACK_CODE;
		lora_transmit(&lora, &tmp, 1);
		ota_transfer_rdy = 1;
	}
	rx_ready = 1;
	printf("rx_buf[0]: %x, rx_ready: %d, ota_transfer_rdy: %d\r\n", rx_buf[0], rx_ready, ota_transfer_rdy);
}
