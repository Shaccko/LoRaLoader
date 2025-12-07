#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <LoRa_raspi.h>
#include <signal.h>

#define CHUNK_SIZE 64 

volatile sig_atomic_t stop = 0;

static void generate_packet(uint8_t* buf, size_t len);
void handle_sigint(int sigint);

/* Program execution:
 * Send half of bin file, 
 * Perform packet validation on chunks received,
 * if invalid, send a BAD_PACKET message to Tx, 
 * download restarts, or exists. If successful, store in sram, 
 * continue transferring other half, validate last half, repeat 
 * previous operation.
 *
 * After a successful transfer, give user option to restart 
 * MCU for firmware upload, swap firmware from sram.
 */

int main() {
	signal(SIGINT, handle_sigint); 

	spidev_init();
	gpio_alloc();

	struct lora lora;

	open_spidev();
	uint8_t status = new_lora(&lora);
	if (status) 
		printf("LoRa detected\n");

	FILE *fp = fopen("firmware.bin", "rb");
	if (!fp) {
		perror("fopen failed\n");
		return 1;
	}

	/* Read total file size */
	uint8_t buf[CHUNK_SIZE];
	size_t bytes_read;

	uint32_t total = 0; 
	uint8_t counter[2]; 
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		lora_transmit(&lora, (uint8_t*)&bytes_read, 1);
		total += bytes_read;
	}
	printf("Total bytes on raspi: %d\n", total);


	//uint32_t counter = 0;
	//while (!stop) {
		//if (lora_transmit(&lora, (uint8_t*)"Hello", 7))
			//printf("Transmitted LoRa message %d\n", counter++);

		//usleep(500*1000);
	//}
	
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

void handle_sigint(int sig) {
	(void)sig;
	stop = 1;
}

static void generate_packet(uint8_t* buf, size_t len) {
	size_t i;
	uint8_t checksum = 0;

	/* Checksum buffer contents */
	for (i = 0; i < len; i++) 
		checksum ^= buf[i];
	memmove(&buf[1], &buf[0], len);

	/* Put len [0], checksum [len - 1] */
	buf[0] = (uint8_t)len;
	buf[len + 1] = checksum;
}	
