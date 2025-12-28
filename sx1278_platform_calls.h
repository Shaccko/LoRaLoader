#pragma once

#include <sx1278_fsk.h>
#ifdef __linux
#include <spi_raspi.h>
#include <packet_transmitter.h>
#include <unistd.h>
#else
#include <spi_stm32.h>
#include <rcc.h>
#include <exti.h>
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

static inline uint32_t get_platform_tick_call(void) {
#ifdef __linux
	return (uint32_t) get_raspi_tick();
#else
	return get_stm32_tick();
#endif
}

static inline void delay_platform_call(uint32_t sleep) {
#ifdef __linux
	usleep(sleep);
#else
	delay(sleep);
#endif
}

static inline void sx1278_set_platform_fsk_pins(void) {

#ifdef __linux
	gpio_raspi_set_mode(RST_PIN, PIN_OUTPUT);
	gpio_raspi_write_pin(RST_PIN, PIN_SET);

	/* Call reset on start */
	gpio_raspi_write_pin(RST_PIN, PIN_RESET);
	usleep(150);
	gpio_raspi_write_pin(RST_PIN, PIN_SET);
	usleep(10 * 1000);
#else
	/* CS, RST */
	gpio_set_mode(CS_PIN|RST_PIN, GPIO_MODE_OUTPUT, SX1278_PORT);
	gpio_write_pin(SX1278_PORT, CS_PIN|RST_PIN, GPIO_PIN_SET); 
	
	/* EXTI */
	gpio_set_mode(IRQ_PIN, GPIO_MODE_INPUT, SX1278_PORT);
	gpio_set_pupdr(IRQ_PIN, PULL_DOWN, SX1278_PORT);
	enable_line_interrupt(IRQ_PIN, SX1278_PORT, RISING); /* EXTI Config */

	/* Call reset on start */
	gpio_write_pin(SX1278_PORT, RST_PIN, GPIO_PIN_RESET);
	delay(1);
	gpio_write_pin(SX1278_PORT, RST_PIN, GPIO_PIN_SET);
	delay(10);
#endif
}

