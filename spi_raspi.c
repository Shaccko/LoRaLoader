#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

//#include <spi_raspi.h>

int main() {
	/* Coming from a bare-metal stm32 background, apparently
	 * just enabling peripheral and sending ioctl with our 
	 * struct msg buffer to spi is enough to get it to function
	 *
	 * ioctl(device, register_request, reg_value) - typical syntax
	 */
	const char* dev = "/dev/spidev0.0";
	int fd_spi = open(dev, O_RDWR);

	if (fd_spi < 0) {
		perror("SPI device failed\n");
		return 1;
	}
	
	uint32_t speed = 1000000;
	uint8_t bits = 8;
	uint8_t mode = SPI_MODE_0;

	/* Set to Mode 0 */
	ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);

	/* Max speed */
	ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

	/* Bits */
	ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);

	const char* msg_buf = "Hello";
	uint64_t msg_rx_buf[10];
	uint32_t msg_len = sizeof(msg_buf);

	struct spi_ioc_transfer packet = {
		.tx_buf = (unsigned long) msg_buf,
		.rx_buf = (unsigned long) msg_rx_buf,

		.len = msg_len,
		.speed_hz = (uint32_t) speed,

		.bits_per_word = bits,
		.cs_change = 0,
		.delay_usecs = 0
	};

	for(;;) {
		ioctl(fd_spi, SPI_IOC_MESSAGE(1), &packet);
		usleep(50 * 1000);
	}

	close(fd_spi);

	return 0;
}

