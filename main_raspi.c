#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <lora_raspi.h>
#include <signal.h>

#define CHUNK_SIZE 200
#define SYNC_BYTE 0xAD
#define ACK_CODE 0xCF 

volatile sig_atomic_t stop = 0;
struct packet {
	uint8_t header;
	uint8_t chunk_size;
	uint8_t chunk_num;
	uint8_t data[CHUNK_SIZE];
	uint8_t checksum;
};

void handle_sigint(int sigint);
static void exit_app(void);

int main() {
	signal(SIGINT, handle_sigint); 

	spidev_init();
	gpio_alloc();
	struct lora lora;

	open_spidev();
	uint8_t status = new_lora(&lora);
	if (status) printf("lora detected\n");
	lora_set_mode(&lora, RXCONT);

	FILE *fp = fopen("firmware.bin", "rb");
	if (!fp) {
		perror("fopen failed\n");
		return 1;
	}

	/* Read total file size */
	/* lora packets to be sent out */
	uint8_t buf[CHUNK_SIZE];
	memset(buf, 0, CHUNK_SIZE);
	uint8_t rx_buf = 0;
	struct packet pkt;

	/* Our chunk data */
	uint32_t total = 0;
	uint8_t chunks_sent = 1;

	size_t bytes_read, i;
	printf("Starting OTA transfer\n");
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		if (stop) exit_app();
		pkt.header = SYNC_BYTE;
		pkt.chunk_size = (uint8_t)bytes_read;
		pkt.chunk_num = chunks_sent++;
		memcpy(pkt.data, buf, bytes_read);
		for (i = 0; i < bytes_read; i++) {
			pkt.checksum ^= buf[i];
		}

		lora_transmit(&lora, (uint8_t*)&pkt, sizeof(pkt));
		/* Wait for ACK from rx */
		total = total + bytes_read;
		lora_set_mode(&lora, RXCONT);
		while ((ACK_CODE & rx_buf) != 0xCF)
			lora_receive(&lora, &rx_buf);
		printf("ACK received, next chunk...\n");

		usleep(1);
	}
	printf("File transfer complete, confirming checksum/size...\n");
	//lora_transmit(&lora, firmware_validate, 2);
	
	/* Wait for final ack */
	lora_set_mode(&lora, SLEEP); /* Finished lora operations. */

	printf("Total bytes on raspi: %d\n", total);

	fclose(fp);
	close_spidev();
	return 0;
}

void handle_sigint(int sig) {
	(void)sig;
	stop = 1;
}

static void exit_app(void) {
	close_spidev();
}
