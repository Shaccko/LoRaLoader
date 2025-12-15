#ifndef SPI_H__
#define SPI_H__
#include <stdint.h>
#include <stddef.h>


#define SPI1 ((struct spi*) (0x40013000))
#define SPI2 ((struct spi*) (0x40003800))
#define SPI3 ((struct spi*) (0x40003C00))
#define SPI4 ((struct spi*) (0x40013400))
#define SPI5 ((struct spi*) (0x40015000))

#define SPI_MASTER (BIT(2))
#define SPI_ENABLE (BIT(6))
#define SPI_8BITMODE (0 << 11)
#define SPI_CLKMODE2 ((BIT(1) | (0)))
#define SPI_CLKMODE0 (0)
#define SPI_MSBFIRST (0)
#define SPI_NSS_SOFT (BIT(9) | BIT(8))
#define SPI_BAUDRATE2 (0)
#define SPI_TIMODE (BIT(4))
#define SPI_TXEIE (BIT(7))
#define SPI_RXNEIE (BIT(6))
#define SPI_BIDIMODE0 (0)
#define SPI_SSOE (BIT(2))

#define SPI_TXE_FLAG (BIT(1))
#define SPI_RXE_FLAG (BIT(0))

#define SPI1_PORT 'A'
#define SCK1_PIN (PIN_NUM(5))
#define NSS1_PIN (PIN_NUM(4)) /* Not used */
#define MOSI1_PIN (PIN_NUM(6))
#define MISO1_PIN (PIN_NUM(7))


struct spi {
	volatile uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR;
};

extern struct spi* spi1;

void spi_init(struct spi* spi, uint32_t spi_pins, uint8_t spi_port);
void spi_write_buf(struct spi* spi, uint8_t* buf);
uint32_t spi_receive_byte(struct spi* spi);
void spi_transmit_data(struct spi* spi, uint8_t* buf, size_t len);
void spi_transmit_receive(struct spi* spi, uint8_t* tx_buf, uint8_t* rx_buf, size_t tx_len);
void spi1_init(void);

#endif
