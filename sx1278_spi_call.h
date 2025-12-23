#pragma once

#include <sx1278_fsk.h>
#ifdef __linux
#include <spidev_raspi.h>
#else
#include <spi_stm32.h>
#endif

static inline void platform_spi_call(void* sx_spi, uint8_t* tx_buf, uint8_t* rx_buf, size_t tx_len) {
#ifdef __linux
	(void) sx_spi;
	spidev_transmit_receive(tx_buf, rx_buf, tx_len);
#else
	gpio_write_pin(SX1278_PORT, CS_PIN, GPIO_PIN_RESET);
	spi_transmit_receive((struct spi*)sx_spi, tx_buf, rx_buf, tx_len);
	gpio_write_pin(SX1278_PORT, CS_PIN, GPIO_PIN_SET);
#endif
}
