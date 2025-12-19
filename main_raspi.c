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

static void handle_sigint(int sig) {
	(void)sig;
	stop = 1;
}

int main(int argc, char *argv[]) {
	if (argc == 1) return 0;

	signal(SIGINT, handle_sigint); 

	spidev_init();
	gpio_alloc();

	open_spidev();
	uint8_t status = new_lora(&lora);
	if (status) printf("lora detected\n");
	lora_set_mode(&lora, RXCONT);

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		perror("fopen failed");
		return 1;
	}

		/* Our chunk data */
	uint32_t total = 0;

	/* Send TX_START ACK */
	/* Busy poll, we want MCU to reset to bootloader and send an ACK
	 * that packets are ready to be received */
	printf("Sending OTA code\n");
	uint8_t tmp = MAGIC_OTA_BYTE;
	uint8_t irq;
	lora_transmit(&lora, &tmp, 1);
	while (rx_buf != ACK_CODE) {
		lora_read_reg(RegIrqFlags, &irq);
		usleep(500 * 1000);
		if (irq & 0x40U) lora_receive(&lora, &rx_buf);
	}
	printf("MCU inside bootloader, sending packets...\n");

	/* lora packets to be sent out */
	uint8_t buf[CHUNK_SIZE];
	memset(buf, 0, CHUNK_SIZE);
	size_t bytes_read;
	printf("Starting OTA transfer\n");
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		if (stop) {
			fclose(fp);
			close_spidev();
			return 0;
		}

		struct packet pkt;
		generate_firmware_packet(&pkt, buf, bytes_read);
		lora_transmit(&lora, (uint8_t*)&pkt, CHUNK_SIZE + 1);
		total = total + bytes_read;
	}
	printf("File transfer complete");
	tmp = PKT_COMPLETE;
	lora_transmit(&lora, &tmp, 1);
	
	lora_set_mode(&lora, SLEEP); /* Finished lora operations. */

	printf("Total bytes on raspi: %d\n", total);

	fclose(fp);
	close_spidev();
	return 0;
}



