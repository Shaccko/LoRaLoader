#ifndef __SPI_RASPI_H__
#define __SPI_RASPI_H__

#include <stdint.h>
#include <linux/spi/spidev.h>

#define SPI_MODE (SPI_MODE_0)
#define SPI_BITS 8
#define SPI_SPEED 10000000

int spidev_init(void);
int spidev_transmit_receive(uint8_t* tx_buf, uint8_t tx_len, uint8_t* rx_buf);


#endif
