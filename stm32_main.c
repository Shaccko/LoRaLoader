#include <stdint.h>
#include <stdio.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>
#include <LoRa_stm32.h>
#include <spi_stm32.h>

struct lora lora;
uint8_t rx_buf[32];

int main() {
	uart2_init();
	spi1_init();
	systick_init();

	/* Things for IRQs:
	 * SYSCFG_EXTICRx
	 * Config port-pin in input mode
	 */

	for(;;) {
		uint8_t status = new_lora(&lora);
		if (status) printf("LoRa detected\r\n");

		delay(500);
	}

}

void lora_rx_irq(void) {
	lora_receive(&lora, rx_buf);
}
