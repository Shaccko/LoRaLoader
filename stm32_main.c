#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>
#include <LoRa_stm32.h>
#include <spi_stm32.h>
#include <exti.h>

struct lora lora;
uint8_t rx_buf[66];
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
	uint32_t bytes_sum = 0;
	for(;;) {
		if (rx_ready) {
			if (strcmp((char*) rx_buf, "DONE")) {
				printf("bytes_sum = %d\r\n", bytes_sum);
			}
			else {
				bytes_sum = bytes_sum + rx_buf[0];	
			}

			rx_ready = 0;
		}
	}

}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
	rx_ready = 1;
}
