#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <LoRa_raspi.h>


int main() {

	gpio_alloc();
	spidev_init();

	struct lora lora;

		open_spidev();
		uint8_t status = new_lora(&lora);
		if (status) {
			printf("LoRa detected\n");
		}
		else {
			printf("Error\n");
		}
		close_spidev();
	
	/*
	 * int fd_bin = open(arg[1], O_RDONLY);
	 * if (fd_bin < 0) {
	 * 	perror("Open failed\n");
	 * 	return -1;
	 * }
	 *
	 * while (contents) {
	 * 	LoRa_transmit(
	 */

	return 0;
}
