#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <packet_transmitter.h>

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

	open_spidev();

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		perror("fopen failed");
		return 1;
	}

	uint8_t buf[CHUNK_SIZE];
	size_t bytes_read;
	while ((bytes_read = fread(buf, 1, CHUNK_SIZE, fp)) > 0) {
	}
	
	fclose(fp);
	close_spidev();
	return 0;
}
