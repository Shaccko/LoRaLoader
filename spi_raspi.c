#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <spi_raspi.h>

int fd_spi;

int spidev_init(void) {
	/* Coming from a bare-metal stm32 background, apparently
	 * just enabling peripheral and sending ioctl with our 
	 * struct msg buffer to spi is enough to get it to function
	 *
	 * ioctl(device, register_request, reg_value) - typical syntax
	 */
	if (open_spidev() < 0) {
		perror("SPI device failed");
		return -1;
	}
	
	
	uint32_t speed = SPI_SPEED;
	uint8_t bits = SPI_BITS;
	uint8_t mode = SPI_MODE;

	/* Set to Mode 0 */
	ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);

	/* Max speed */
	ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	ioctl(fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

	/* Bits */
	ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);

	return 1;
}

int spidev_transmit_receive(uint8_t* mosi_buf, uint8_t* miso_buf,  size_t mosi_len) {
	struct spi_ioc_transfer packet = { 
		.tx_buf = (unsigned long) mosi_buf,
		.rx_buf = (unsigned long) miso_buf,

		.len = mosi_len,
		.speed_hz = SPI_SPEED,
		.cs_change = 0,

		.bits_per_word = 8,
		//.delay_usecs = 2,
		//.word_delay_usecs = 2
	};

	if (ioctl(fd_spi, SPI_IOC_MESSAGE(1), &packet) < 0) {
		perror("Error in SPI transmission");
		return -1;
	}
	return 1;
}

int open_spidev(void) {
	fd_spi = open("/dev/spidev0.0", O_RDWR);

	if (fd_spi < 0) return -1;

	return 1;
}

void close_spidev(void) {
	close(fd_spi);
}


