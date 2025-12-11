#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <lora_raspi.h>
#include <packet_transmitter.h>

volatile sig_atomic_t stop = 0;

struct lora lora;
uint8_t rx_buf;

static void handle_sigint(int sigint);

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

	/* Send TX_START ACK */
	printf("Sending OTA code\n");
	uint8_t tmp = OTA_TX_START;
	if (send_tx_wait_ack(&lora, &tmp, 1) != 1) {
		printf("Error in tx, exiting...\n");
		return 0;
	}

	size_t bytes_read;
	printf("Starting OTA transfer\n");
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		if (stop) {
			fclose(fp);
			close_spidev();
			return 0;
		}

		/* Store files position incase of an ack error */
		//long int old_file_pos = ftell(fp);
		generate_firmware_packet(&pkt, buf, bytes_read);
		if (send_tx_wait_ack(&lora, (uint8_t*)&pkt, bytes_read + 4) != 1) {
			printf("Failed packet transmission, exiting...\n");
			return 0;
			//fseek(fp, old_file_pos, SEEK_SET);
		}
		else {
			printf("ACK received, next chunk...\n");
			total = total + bytes_read;
			increment_chunk_num();
		}
		usleep(1);
	}
	printf("File transfer complete, confirming checksum/size...\n");
	tmp = PKT_COMPLETE;
	lora_transmit(&lora, &tmp, 1);
	
	/* Wait for final ack */
	lora_set_mode(&lora, SLEEP); /* Finished lora operations. */

	printf("Total bytes on raspi: %d\n", total);

	fclose(fp);
	close_spidev();
	return 0;
}

static void handle_sigint(int sig) {
	(void)sig;
	stop = 1;
}


