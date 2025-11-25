#ifndef __SPI_DEV_H__
#define __SPI_DEV_H__


#define SPI0 ((struct spi*) 0x7E204000)

struct spi {
	volatile uint32_t CS, FIFO, CLK, DLEN, LTOH, DC;
};


#endif
