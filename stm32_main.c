#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>
#include <LoRa_stm32.h>
#include <spi_stm32.h>
#include <exti.h>
#include <packet_parser.h>

struct lora lora;
uint8_t rx_buf[CHUNK_SIZE];
uint8_t rx_ready = 0;

int main() {
	uart2_init();
	spi1_init();
	systick_init();

	uint8_t status = new_lora(&lora);
	if (status) {
		printf("LoRa detected\r\n");
	}
	lora_set_mode(&lora, RXCONT);

	uint8_t firmware_info[2] = { 0U, 0U }; /* [0] checksum, [1] chunk rx'd */
	uint8_t debug_counter = 0;

	memset(rx_buf, 0, sizeof(rx_buf));
	for(;;) {
		if (rx_ready) {
			/* Send an ACK back to tx to confirm halfway and end pointpacket validation */
			if (rx_buf[2] == ACK_CODE) {
				printf("rx_buf[2]: %x\r\n", rx_buf[2]);
				uint8_t ack_status = validate_packets_received(firmware_info, rx_buf);
				lora_transmit(&lora, &ack_status, 1);
				debug_counter++;
			}
			process_firmware_packet(firmware_info, rx_buf);

			rx_ready = 0;
		}

		if (debug_counter == 2) 
			printf("checksum: %x, chunks rx'd: %d\r\n", firmware_info[0], firmware_info[1]);
	}
}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
