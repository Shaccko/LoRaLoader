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

#define MAGIC_BYTE 0xCE

struct lora lora;
uint8_t rx_buf[CHUNK_SIZE + 4]; /* Header + CHUNK_SIZE */
uint8_t rx_ready = 0;

__attribute__((section(".magic_ota_byte"))) volatile uint8_t magic_byte = 0;

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
	int ack_timeout = 0;
	for(;;) {
		if (rx_ready) {
			if (rx_buf[0] == MAGIC_BYTE) {
				magic_byte = MAGIC_BYTE;
				printf("OTA firmware detected, reset MCU to load new firmware.\r\n");
			}
			rx_ready = 0;
		}
				
		/*
		delay(1);
		if (rx_ready) {
			switch (rx_buf[0] & 0xFF) {
				case (OTA_TX_START):
				case (OTA_BYTE):
				case (PKT_COMPLETE):
					parse_packet_state(&lora, rx_buf, &out_pkt);
					ack_timeout = (int) get_tick();
					break;
			}
			rx_ready = 0;
		}
		if ((get_ota_state() == 1) && (((int) get_tick() - ack_timeout) > 5000)) {
			kill_ota_firmware();
		}
		*/
	}
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
