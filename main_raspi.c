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
	sx1278_write_reg(RegFifoThresh, 0x1);
	fsk_set_payload_len(MAX_FIFO_CHUNK);
	sx1278_set_mode(RXCONT);

	uint8_t buf[CHUNK_SIZE];
	uint32_t counter = 0;
	while(stop == 0) {
		fsk_receive(buf);
		if (strcmp((char*)buf, "This is a transmission") != 0) {
			counter++;
			printf("%d\r\n", counter);
		}
	}
	size_t bytes_read;
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
	}
	
	fclose(fp);
	close_spidev();
	return 0;
}
