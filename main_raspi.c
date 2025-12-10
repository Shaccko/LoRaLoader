#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <lora_raspi.h>
#include <signal.h>

#define ACK_CODE 0xAC
#define OTA_BYTE 0xBC
#define OTA_TX_START 0xCC
#define OTA_TX_STOP 0xDC
#define PKT_PASS 0xEC

#define CHUNK_SIZE 200

volatile sig_atomic_t stop = 0;
struct packet {
	uint8_t header;
	uint8_t chunk_size;
	uint8_t chunk_num;
	uint8_t checksum;
	uint8_t data[CHUNK_SIZE];
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

	/* Send TX_START ACK */
	printf("Sending OTA code\n");
	uint8_t tmp = OTA_TX_START;
	lora_transmit(&lora, &tmp, 1);
	if (wait_ack() != 1) {
		perror("Failed ack, exiting...\n");
		return 0;
	}

	size_t bytes_read, i;
	printf("Starting OTA transfer\n");
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		if (stop) exit_app();
		
		pkt.header = OTA_BYTE;
		pkt.chunk_size = (uint8_t)bytes_read;
		pkt.chunk_num = chunks_sent++;
		memcpy(pkt.data, buf, bytes_read);
		pkt.checksum = 0;
		for (i = 0; i < bytes_read; i++) {
			pkt.checksum ^= buf[i];
		}

		lora_transmit(&lora, (uint8_t*)&pkt, bytes_read + 4);
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

	do {
		lora_read_reg(RegIrqFlags, &irq);
		usleep(5);
	} while ((irq & 0x40U) == 0);
	lora_receive(&lora, &rx_buf);

	if (rx_buf == ACK_CODE || rx_buf == PKT_PASS) {
		return 1;
	}

	return 0;
}

static void exit_app(void) {
	close_spidev();
}
