#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <spi_raspi.h>

int spidev_init(void) {
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
	
	uint32_t speed = SPI_SPEED;
	uint8_t bits = SPI_BITS;
	uint8_t mode = SPI_MODE;

	/* Set to Mode 0 */
	ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);

	/* Max speed */
	ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

	/* Bits */
	ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);


	close(fd_spi);

	return 1;
}

int spidev_transmit_receive(uint8_t* tx_buf, uint8_t* rx_buf,  size_t tx_len) {
	const char* dev = "/dev/spidev0.0";
	int fd_dev = open(dev, O_RDWR);
	if (fd_dev < 0) {
		perror("SPI open failed\n");
		return -1;
	}

	struct spi_ioc_transfer packet = {
		.tx_buf = (unsigned long) tx_buf,
		.rx_buf = (unsigned long) rx_buf,

		.len = tx_len,
		.speed_hz = (uint32_t) SPI_SPEED,

		.bits_per_word = SPI_BITS,
		.cs_change = 0,
		.delay_usecs = 0
	};

	if (ioctl(fd_dev, SPI_IOC_MESSAGE(1), &packet) < 0) {
		perror("Error in SPI transmission\n");
		return -1;
	}

	close(fd_dev);
	return 1;
}


