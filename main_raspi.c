#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <LoRa_raspi.h>
#include <signal.h>

#define CHUNK_SIZE 200
#define ACK_CODE 0xCF

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
	if (status) printf("LoRa detected\n");
	lora_set_mode(&lora, STDBY);

	FILE *fp = fopen("firmware.bin", "rb");
	if (!fp) {
		perror("fopen failed\n");
		return 1;
	}

	/* Get file's halfway point */
	fseek(fp, 0, SEEK_END);
	uint32_t half_size = (uint32_t)ftell(fp) >> 1;
	rewind(fp);

	/* Read total file size */
	/* lora packets to be sent out */
	uint8_t buf[CHUNK_SIZE];
	uint8_t firmware_validate[3];
	uint8_t rx_buf;

	/* Our chunk data */
	uint32_t half_chunks = half_size / CHUNK_SIZE;
	uint32_t total = 0;
	uint8_t chunks_sent;
	uint8_t checksum = 0;

	size_t bytes_read, i;
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		if (chunks_sent == half_chunks) {
			printf("Halfway point reached\n");
			printf("Chunks sent: %d, transmitting checksum and size\n", chunks_sent);
			firmware_validate[0] = checksum;
			firmware_validate[1] = chunks_sent;
			firmware_validate[2] = ACK_CODE;
			lora_transmit(&lora, firmware_validate, 3);

			printf("Waiting for ack...\n");
			lora_set_mode(&lora, RXCONT);
			while ((ACK_CODE & rx_buf) != 0xCF)
				lora_receive(&rx_buf);
			lora_set_mode(&lora, STDBY); 
			 	
			printf("Ack received, transferring rest.\n");
		}
		printf("buf[2] = %x\n", buf[2]);
		lora_transmit(&lora, buf, bytes_read);

		/* A simple XOR checksum */
		for (i = 0; i < bytes_read; i++) {
			checksum ^= buf[i];
		}
		total = total + bytes_read;
		chunks_sent++;
		printf("Checksum: %x, chunks_sent: %d\n", checksum, chunks_sent);

		usleep(1*1000);
	}
	printf("File transfer complete, confirming checksum/size...\n");
	firmware_validate[0] = checksum;
	firmware_validate[1] = chunks_sent;
	lora_transmit(&lora, firmware_validate, 2);
	
	/* Wait for final ack */
	lora_set_mode(&lora, RXCONT);
	while ((ACK_CODE & rx_buf) != 0xCF)
	 	lora_receive(&rx_buf);
	lora_set_mode(&lora, SLEEP); /* Finished lora operations. */

	printf("Total bytes on raspi: %d\n", total);

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
	buf[0] = checksum;
	buf[len + 1] = (uint8_t)len;
}	
