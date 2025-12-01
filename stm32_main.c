#include <stdint.h>
#include <stdio.h>

#include <rcc.h>
#include <hal.h>
#include <uart.h>
#include <LoRa_stm32.h>
#include <spi_stm32.h>


int main() {
	uart2_init();
	spi1_init();
	systick_init();

	struct lora lora;

	for(;;) {
		uint8_t status = new_lora(&lora);
		if (status) printf("LoRa detected\r\n");

		printf("Done execution\r\n");
		delay(500);
	}

}
