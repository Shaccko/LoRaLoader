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
#define PKT_PASS 0xCF 

volatile sig_atomic_t stop = 0;
static uint8_t ack_code = 0xAC;
struct packet {
	uint8_t header;
	uint8_t chunk_size;
	uint8_t chunk_num;
	uint8_t data[CHUNK_SIZE];
	uint8_t checksum;
};

struct lora lora;
uint8_t rx_buf;

void handle_sigint(int sigint);
static void exit_app(void);
static uint8_t wait_ack(void);

int main() {
	signal(SIGINT, handle_sigint); 

	spidev_init();
	gpio_alloc();

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
	struct packet pkt;

	/* Our chunk data */
	uint32_t total = 0;
	uint8_t chunks_sent = 1;

	size_t bytes_read, i = 5;
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		if (i == 9) {
			return 0;
			fclose(fp);
		}
		buf[0] = i++;
		printf("buf[0]: %d\n", buf[0]);
		lora_transmit(&lora, buf, 20);
	}
	printf("Sending OTA code\n");
	lora_transmit(&lora, &ack_code, 1);
	if (wait_ack() != 1) {
		perror("Failed ack, exiting...\n");
		return 0;
	}
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
		printf("Sent transmission\n");

		/* Wait for ACK from rx */
		printf("Waiting for ack...\n");
		if (wait_ack() != 1) {
			perror("Failed ack, exiting...\n");
			return -1;
		}
		printf("ACK received, next chunk...\n");
		total = total + bytes_read;

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

static inline uint8_t wait_ack(void) {
	uint8_t irq;
	lora_set_mode(&lora, RXCONT);

	do {
		lora_read_reg(RegIrqFlags, &irq);
		usleep(1);
	} while ((irq & 0x40) == 0);
	lora_receive(&lora, &rx_buf);
	printf("%x\n", rx_buf);

	if (rx_buf != ack_code) {
		return 0;
	}

	return 1;
}

static void exit_app(void) {
	close_spidev();
}
