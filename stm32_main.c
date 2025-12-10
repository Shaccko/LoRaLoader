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
uint8_t rx_ready = 0, firmware_ota_ready = 0;
static uint8_t ack_code = 0xAC;

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
	uint8_t buf[100];
	memset(buf, 0, 100);

	for (int i = 0; i < 2; i++) {
		lora_transmit(&lora, buf, 100);
	}

	for(;;) {
		if (rx_ready) {
			printf("rx[0] = %d\r\n", rx_buf[0]);
			rx_ready = 0;
		}
		if (firmware_ota_ready) {
			delay(1);
			printf("Received OTA code\r\n");
			uint8_t pkt_status = validate_packets_received(rx_buf, &out_pkt);
			lora_transmit(&lora, &pkt_status, 1);
			firmware_ota_ready = 0;
		}
	}
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	if (*rx_buf == ACK_CODE) {
		lora_transmit(&lora, ack_code, 1);
		firmware_ota_ready = 1;
	}
	else {
		rx_ready = 1;
	}
}
