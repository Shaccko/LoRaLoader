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
#include <sx1278_fsk.h>

uint8_t rx_buf[CHUNK_SIZE + 4]; /* Header + CHUNK_SIZE */
volatile uint8_t rx_ready = 0;

int main() {
	uart2_init();
	spi1_init();
	systick_init();

	if (init_fsk() == OK) printf("SX1278 Detected\r\n");

	sx1278_set_mode(RX);
	uint32_t timer = 0;
	for(;;) {
		delay(1);
		if (rx_ready) {
			write_packet(rx_buf);
			if (rx_buf[0] == OTA_PACKET_BYTE) {
				timer = get_stm32_tick();
			}

		}

		/* Packet timeout */
		if (get_ota_state() == 1 && ((get_stm32_tick() - timer) > 5000)) {
			kill_ota_firmware();
		}

		rx_ready = 0;
	}
}

void sx1278_rx_irq(void) {
	fsk_receive(rx_buf);
	rx_ready = 1;
}
