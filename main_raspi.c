#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <packet_transmitter.h>
#include <sx1278_fsk.h>

volatile sig_atomic_t stop = 0;
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

	if (init_fsk() != 1) {
		printf("Failed FSK init\n");
		return 1;
	}

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		perror("fopen failed");
		return 1;
	}



	/* Image should store:
	 * CRC maybe
	 * vto
	 * Where it wants to place itself in flash
	 * all inside sram
	 */

	/* Send start code, and wait for an 
	 * ack back
	 */
	uint8_t tmp = PKT_START;
	fsk_transmit(&tmp, 1);
	sx1278_set_mode(RX);
	usleep(5);
	fsk_receive(&rx_buf);

	if (rx_buf != ACK_CODE) {
		printf("Failed receiving ack...\r\n");
		return 0;
	}
	
	uint8_t buf[CHUNK_SIZE + 1];
	size_t bytes_read;
	uint32_t total = 0;
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
		struct image_packet pkt;
		generate_firmware_packet(&pkt, buf, bytes_read);
		fsk_transmit((uint8_t*)&pkt, bytes_read + 1);
		total += bytes_read;
		usleep(1);
	}

	printf("total: %d\r\n", total);

	uint8_t stop_code = PKT_COMPLETE;
	fsk_transmit(&stop_code, 1);
	
	printf("done\n");
	
	sx1278_set_mode(SLEEP);
	
	fclose(fp);
	close_spidev();
	return 0;
}
