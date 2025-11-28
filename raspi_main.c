#include <stdint.h>
#include <stdio.h>

#include <LoRa_raspi.h>


int main() {
	
	struct lora lora;
	if (new_lora(&lora)) {
		printf("LoRa detected\n");
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

	return 0;
}
