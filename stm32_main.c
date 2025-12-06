#include <stdint.h>
#include <stdio.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>
#include <LoRa_stm32.h>
#include <spi_stm32.h>
#include <exti.h>

struct lora lora;
uint8_t rx_buf[32];
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
	for(;;) {
		if (rx_ready) {
			printf("rx_buf: %s\r\n", rx_buf);
			rx_ready = 0;
		}

	//	lora_transmit(&lora, (uint8_t*)"Hello", 5);
		printf("Done loop execution\r\n");
		delay(500);
	}

}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
