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

#define CRC ((struct crc*) 0x40023000)

struct crc {
	volatile uint32_t DR, IDR, CR;
};

uint8_t rx_buf[CHUNK_SIZE + 4]; /* Header + CHUNK_SIZE */
volatile uint8_t rx_ready = 0;

__attribute__((section(".magic_ota_byte"))) volatile uint8_t magic_byte = 0;

int main() {
	extern uint32_t __image_curr;
	uart2_init();
	spi1_init();
	systick_init();

	if (init_fsk() == OK) printf("SX1278 Detected\r\n");

	/* USE FSK FOR PACKETS */
	//fsk_transmit((uint8_t*)'f', 1);
	uint32_t counter = 0;
	for(;;) {
		if (rx_ready) {
			rx_ready = 0;
		}
		//printf("Transmitted.\r\n");
		delay(1000);
	}
}

void sx1278_rx_irq(void) {
	fsk_receive(rx_buf);
	rx_ready = 1;
}
