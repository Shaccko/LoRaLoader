#include <stdint.h>
#include <spi.h>
#include <hal.h>
#include <uart.h>


struct spi* spi1 = SPI1;

void spi_init(struct spi* spi, uint32_t spi_pins, uint8_t spi_port) {
	/* Define SCL baud rate, SPI_CR1
	 * Select CPOL and CPHA bits for relationship type
	 * and other stuff
	 */

	uint8_t spi_af = 5;
	/* SPI1 config only for now */

	if (spi == SPI1) RCC->APB2ENR |= BIT(12); /* SPI1 Clock */

	gpio_set_mode(spi_pins, GPIO_MODE_AF, spi_port);
	gpio_set_af(spi_pins, spi_af, spi_port);
	gpio_set_speed(spi_pins, HIGH_SPEED, spi_port);



	spi->CR1 = 0;
	spi->CR2 = 0;
	spi->CR1 = SPI_MASTER | SPI_8BITMODE | \
		SPI_CLKMODE0 | SPI_MSBFIRST | SPI_NSS_SOFT | \
		SPI_BAUDRATE2 | SPI_BIDIMODE0;
	spi->CR2 = SPI_TXEIE | SPI_RXNEIE; 
	spi->CR1 |= SPI_ENABLE;
}

uint32_t spi_receive_byte(struct spi* spi) {
	while(!(spi->SR & SPI_RXE_FLAG));
	return spi->DR;
}



void spi_write_buf(struct spi* spi, uint8_t* buf) {
	while(!(spi->SR & SPI_TXE_FLAG)); 
	spi->DR = (uint32_t)*buf;
}

void spi_transmit_data(struct spi* spi, uint8_t* buf, size_t len) {
	while (len--){
		spi_write_buf(spi, buf++);
	}
}

void spi_transmit_receive(struct spi* spi, uint8_t* tx_buf, uint8_t* rx_buf, size_t tx_len) {
	size_t i = 0;
	uint8_t dummy[64];
	if (rx_buf == NULL || rx_buf == 0) rx_buf = dummy;
	while(tx_len--) {
		spi_write_buf(spi, tx_buf++);
		rx_buf[i++] = (uint8_t) spi_receive_byte(spi);
	}
}

void spi1_init(void) {
	uint32_t spi1_pins = SCK1_PIN | MOSI1_PIN | MISO1_PIN | NSS1_PIN;
	uint8_t spi1_port = SPI1_PORT;
	spi_init(spi1, spi1_pins, spi1_port);
}

