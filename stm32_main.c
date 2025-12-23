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

__attribute__((section(".magic_ota_byte"))) volatile uint8_t magic_byte = 0;

int main() {
	uart2_init();
	spi1_init();
	systick_init();

	if (init_fsk() != OK) printf("SX1278 Failed.\r\n");

	/* USE FSK FOR PACKETS */
	for(;;) {
		fsk_transmit((uint8_t*) "Hello", 5);
		printf("Transmitted.\r\n");
		delay(1000);
	}
}

void lora_rx_irq(void) {
	//lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
