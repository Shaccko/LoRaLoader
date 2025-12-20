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

struct fsk fsk;
uint8_t rx_buf;

static void handle_sigint(int sig) {
	(void)sig;
	stop = 1;
}

int not_in_use(int argc, char *argv[]) {
	if (argc == 1) return 0;

	signal(SIGINT, handle_sigint); 

	spidev_init();
	gpio_alloc();

	open_spidev();
	uint8_t status = new_fsk(&fsk);
	if (status) printf("lora detected\n");
	set_mode(RXCONT);

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
	//lora_transmit(&lora, &tmp, 1);
	while (rx_buf != ACK_CODE) {
		read_reg(RegIrqFlags, &irq);
		usleep(50 * 1000);
		//if (irq & 0x40U) lora_receive(&lora, &rx_buf);
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
		//lora_transmit(&lora, (uint8_t*)&pkt, CHUNK_SIZE + 1);
		total = total + bytes_read;
	}
	printf("File transfer complete");
	tmp = PKT_COMPLETE;
	//lora_transmit(&lora, &tmp, 1);
	
	set_mode(SLEEP); /* Finished lora operations. */

	printf("Total bytes on raspi: %d\n", total);

	fclose(fp);
	close_spidev();
	return 0;
}

int main() {
	spidev_init();
	gpio_alloc();

	open_spidev();
	uint8_t status = new_fsk(&fsk);
	write_reg(RegPacketConfig1, 0x80); /* Fixed length packets */
	uint16_t max_pkt_length = 500;
	write_reg(RegPacketConfig2, (uint8_t) (max_pkt_length >> 8));
	write_reg(RegPayloadLength, (uint8_t) (max_pkt_length >> 0)); /* Setting fixed packet length to 2000 */
	if (status) printf("lora detected\n");
	uint8_t buf[max_pkt_length];

	FILE *fp = fopen("padded.bin", "rb");
	if (!fp) {
		perror("fopen failed");
		return 1;
	}
	memset(buf, 0xA, max_pkt_length);


	size_t bytes_read = 0;
	while ((bytes_read = fread(buf, 1, max_pkt_length, fp)) > 0) {
		fsk_transmit(buf, max_pkt_length);
	}

	set_mode(SLEEP); /* Finished lora operations. */

	close_spidev();
	return 0;
}

