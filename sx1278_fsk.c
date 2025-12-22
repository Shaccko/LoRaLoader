#include <string.h>

#include <lora_stm32.h>
#include <hal.h>
#include <spi_stm32.h>
#include <uart.h>
#include <exti.h>

static uint8_t curr_mode = 0;

/* Streams msg into Fifo, using FifoThresh.
 * If Fifo empty, start filling fifo with bytes,
 * wait till FifoThresh gets set, repeat.
 */
uint8_t fsk_transmit_stream(uint8_t* msg, size_t msg_len) {
	uint8_t fifo_buf[FIFO_CHUNK];

	sx1278_set_mode(TX);

	size_t total_fifo_size = msg_len;
	while (total_fifo_size > 0) {
		uint16_t chunk = (total_fifo_size > FIFO_CHUNK ?
				FIFO_CHUNK : total_fifo_size;

		memcpy(msg, fifo_buf, chunk);
		sx1278_burstwrite_fifo(fifo_buf, chunk);
		total_fifo_size = total_fifo_size - chunk;

		/* Wait for Fifo to be empty */
		while ((get_fifo_status()) != FIFO_EMPTY);
	}
}

/* Kbp\s presets. These bitrates are only as accurate
 * as our software allows us to be, taking account of
 * SPI overhead, preamble, crc, etc. 
 *
 * Making sure to follow:
 * h = (2*fdev)/bitrate, h = modulation index, 1.0 is ideal
 * RxBw >= 2*(fdev+bitrate)
 */

/* h = 0.5 */
void fsk_kbps_fast(void) {
	if (curr_state != STDBY) sx1278_set_mode(STDBY);
	
	/* Setting a kpbs of 80,
	 * fdev around 20KHz, 
	 * RxBw >= 200 
	 */
	uint16_t bitrate = KPBS_80;
	sx1278_write_reg(RegBitrateMsb, (uint8_t) (bitrate >> 8));
	sx1278_write_reg(RegBitrateLsb, (uint8_t) (bitrate >> 0));

	uint16_t fdev_reg = 0x148;
	sx1278_write_reg(RegFdevMsb, fdev_reg >> 8);
	sx1278_write_reg(RegFdevLsb, fdev_reg >> 0);

	uint8_t rxbw_reg = 0x9; 
	sx1278_write_reg(RegBxMant, rxbw_reg);
}

/* h = 0.8 */
void fsk_kbps_mid(void) {
	if (curr_state != STDBY) sx1278_set_mode(STDBY);
	
	/* Setting a kpbs of 50,
	 * fdev around 20KHz, 
	 * RxBw >= 150 
	 */
	uint16_t bitrate = KPBS_50;
	sx1278_write_reg(RegBitrateMsb, (uint8_t) (bitrate >> 8));
	sx1278_write_reg(RegBitrateLsb, (uint8_t) (bitrate >> 0));

	uint16_t fdev_reg = 0x143;
	sx1278_write_reg(RegFdevMsb, fdev_reg >> 8);
	sx1278_write_reg(RegFdevLsb, fdev_reg >> 0);

	uint8_t rxbw_reg = 0x11; 
	sx1278_write_reg(RegBxMant, rxbw_reg);
}

/* h = 1.0 */
void fsk_kbps_slow(void) {
	if (curr_state != STDBY) sx1278_set_mode(STDBY);
	
	/* Setting a kpbs of 10,
	 * fdev around 5KHz, 
	 * RxBw >= 30 
	 */
	uint16_t bitrate = KPBS_10;
	sx1278_write_reg(RegBitrateMsb, (uint8_t) (bitrate >> 8));
	sx1278_write_reg(RegBitrateLsb, (uint8_t) (bitrate >> 0));

	uint16_t fdev_reg = 0xA3;
	sx1278_write_reg(RegFdevMsb, fdev_reg >> 8);
	sx1278_write_reg(RegFdevLsb, fdev_reg >> 0);

	uint8_t rxbw_reg = 0x4; 
	sx1278_write_reg(RegBxMant, rxbw_reg);


void sx1278_write_reg(struct lora* lora, uint8_t addr, uint8_t val) {
	if (curr_state != STDBY) sx1278_set_mode(STDBY);

	uint8_t reg[2];
	static const size_t reg_len = 2;

	reg[0] = 0x80 | addr;
	reg[1] = val;

	gpio_write_pin(LORA_PORT, CS_PIN, GPIO_PIN_RESET);
	spi_transmit_receive(lora->lspi, reg, 0, reg_len);
	gpio_write_pin(LORA_PORT, CS_PIN, GPIO_PIN_SET);
}

void sx1278_burstwrite_fifo(struct lora* lora, uint8_t* payload, size_t payload_len) {
	/* This line will be kept as memorabilia, as it alone
	 * was the cause of a day of debugging why my payloads
	 * were not being received properly. 
	 */
	/* if (payload_len > 33) return; */

	if (curr_state != STDBY) sx1278_set_mode(STDBY);

	uint8_t reg[32];
	size_t reg_len = payload_len + 1;
	reg[0] = 0x80 | RegFifo;
	memcpy(&reg[1], payload, payload_len);

	gpio_write_pin(lora->lora_port, lora->cs_pin, GPIO_PIN_RESET);
	spi_transmit_receive(lora->lspi, reg, (uint8_t*)0, reg_len);
	gpio_write_pin(lora->lora_port, lora->cs_pin, GPIO_PIN_SET);
}


void sx1278_read_reg(struct lora* lora, uint8_t addr, uint8_t* out) {
	if (curr_state != STDBY) sx1278_set_mode(STDBY);

	uint8_t reg[2];
	uint8_t rx_buf[2];
	static const size_t reg_len = 2;

	reg[0] = addr & 0x7F; 
	reg[1] = 0;

	gpio_write_pin(lora->lora_port, lora->cs_pin, GPIO_PIN_RESET);
	spi_transmit_receive(lora->lspi, reg, rx_buf, reg_len);
	gpio_write_pin(lora->lora_port, lora->cs_pin, GPIO_PIN_SET);
	
	*out = rx_buf[1];
}

void sx1278_set_mode(struct lora* lora, uint8_t mode) {
	uint8_t curr_op = 0;

	lora_read_reg(lora, RegOpMode, &curr_op);
	curr_op = (uint8_t) ((curr_op & ~7U) | mode); /* Overwrite mode bits */

	lora_write_reg(lora, RegOpMode, curr_op);
	lora->curr_mode = mode;
}
