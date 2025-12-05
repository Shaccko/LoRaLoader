#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <LoRa_raspi.h>
#include <signal.h>

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
	(void)sig;
	stop = 1;
}

int main() {
	signal(SIGINT, handle_sigint); 

	spidev_init();
	gpio_alloc();

	struct lora lora;

	open_spidev();
	uint8_t status = new_lora(&lora);
	if (status) {
		printf("LoRa detected\n");
	}
	else {
		printf("Error\n");
	}

	uint32_t counter = 0;
	while (!stop) {
		if (lora_transmit(&lora, (uint8_t*)"Hello", 5))
			printf("Transmitted LoRa message %d\n", counter++);

		usleep(500*1000);
	}
	
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

	close_spidev();
	return 0;
}
