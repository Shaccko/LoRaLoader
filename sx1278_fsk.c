#include <string.h>

#include <hal.h>
#include <sx1278_fsk.h>
#include <spi_stm32.h>
#include <uart.h>
#include <exti.h>

#include <sx1278_platform_calls.h>

static uint8_t curr_mode = 0;
void* chip_spi;

static uint8_t wait_irq_flag(uint8_t flag_code);
static inline void sx1278_set_fsk_configs(void);
static inline void fsk_kbps_fast(void);
static inline void fsk_kbps_mid(void);
static inline void fsk_kbps_slow(void);

uint8_t init_fsk(void) {
#ifndef __linux
	chip_spi = (struct spi*)spi1;
#endif
	sx1278_set_mode(SLEEP);	

	/* Pin Configs */
	sx1278_set_platform_fsk_pins();

	/* FSK Configs */
	sx1278_set_fsk_configs();
	fsk_kbps_mid();

	/* Set DIO here */

	sx1278_set_mode(STDBY);	

	uint8_t chip_version;
	sx1278_read_reg(RegVersion, &chip_version);

	return chip_version == 0x12 ? OK : FAIL;
}

/* Streams msg into Fifo, using FifoThresh.
 * If Fifo empty, start filling fifo with bytes,
 * wait till FifoThresh gets set, repeat.
 */
uint8_t fsk_transmit_stream(uint8_t* msg, size_t msg_len) {
	/* FifoThresh, set to 63 since we put 64
	 * bytes into Fifo, and TX triggers on
	 * FIFO >= FifoThresh + 1
	 */
	sx1278_set_mode(STDBY);	
	sx1278_write_reg(RegFifoThresh, 0x3F); /* Start cond at 0 */
	fsk_set_payload_len(MAX_FIXED_CHUNK);

	sx1278_set_mode(TX);
	uint8_t fifo_buf[MAX_FIFO_CHUNK];
	size_t total_fifo_size = msg_len;
	while (total_fifo_size > 0) {
		/* Get 64 or less than 64 depending on which is greater */
		uint16_t chunk = (uint16_t) ((total_fifo_size > MAX_FIFO_CHUNK) ?
				MAX_FIFO_CHUNK : total_fifo_size);

		memcpy(fifo_buf, msg, chunk);
		sx1278_burstwrite_fifo(fifo_buf, chunk);
		total_fifo_size = total_fifo_size - chunk;
		msg = msg + chunk; /* Advancing ptr */

		/* Wait for Fifo to be empty */
		if (wait_irq_flag(FIFO_EMPTY) == 0) {
			return FAIL;
		}
			
	}

	/* Wait for TX to finish */
	if (wait_irq_flag(PACKET_SENT) == 0) {
		return FAIL;
	}

	sx1278_set_mode(STDBY);	

	return OK;
}

/* For less than 64 bytes tranmission, this is preferred */

static void print_regs(void) {
	uint8_t reg;
	sx1278_read_reg(RegPacketConfig2, &reg);
	printf("PConfig2: %x\r\n", reg);
	sx1278_read_reg(RegPacketConfig1, &reg);
	printf("PConfig1: %x\r\n", reg);
	sx1278_read_reg(RegPayloadLength, &reg);
	printf("payload len: %x\r\n", reg);
	sx1278_read_reg(RegFifoThresh, &reg);
	printf("RegFifoThresh: %x\r\n", reg);
}
uint8_t fsk_transmit(uint8_t* msg, size_t msg_len) {
	print_regs();
	if (msg_len > MAX_FIFO_CHUNK) return FAIL;

	/* Change packet format to variable 
	 * Set TxStartCondition to FifoEmpty */
	sx1278_set_mode(STDBY);	

	sx1278_write_reg(RegPacketConfig1, 0x90); /* Variable packet, CRC on */
	sx1278_write_reg(RegFifoThresh, 0x80); /* Start cond at 0 */
	fsk_set_payload_len(MAX_FIFO_CHUNK);

	/* [0] = msg_len ? */
	uint8_t fifo_buf[MAX_FIFO_CHUNK];
	fifo_buf[0] = (uint8_t) msg_len;
	memcpy(&fifo_buf[1], msg, msg_len);
	sx1278_burstwrite_fifo(fifo_buf, msg_len + 1);
	sx1278_set_mode(TX);

	/* Wait for TX to finish */
	if (wait_irq_flag(PACKET_SENT) == 0) {
		return FAIL;
	}

	sx1278_set_mode(STDBY);	

	return OK;
}

uint8_t fsk_receive(uint8_t* rx_buf) {
	print_regs();
	if (wait_irq_flag(PAYLOAD_READY) == 0) {
		return FAIL;
	}
	uint8_t fifo_len;
	sx1278_read_reg(RegFifo, &fifo_len);
	printf("Fifo_len: %x\n", fifo_len);
	if (fifo_len == 0) return FAIL;

	size_t i;
	for (i = 1; i < fifo_len + 1; i++) {
		sx1278_read_reg(RegFifo, &rx_buf[i]);
	}

	sx1278_set_mode(STDBY);
	sx1278_set_mode(RXCONT);
	return OK;
}


/* LNA Gain, PaRamp and Power Gain */
static inline void sx1278_set_fsk_configs(void) {
	sx1278_write_reg(RegGainConfig, POWER_20dB);
	sx1278_write_reg(RegLNA, 0x23);
	sx1278_write_reg(RegPaRamp, 0xF);
}	

static inline void sx1278_set_fsk_pins_stm32(void) {
	gpio_set_mode(CS_PIN|RST_PIN, GPIO_MODE_OUTPUT, SX1278_PORT);
	gpio_set_mode(IRQ_PIN, GPIO_MODE_INPUT, SX1278_PORT);
	gpio_set_pupdr(IRQ_PIN, PULL_DOWN, SX1278_PORT);
	gpio_write_pin(SX1278_PORT, CS_PIN|RST_PIN, GPIO_PIN_SET); 

	enable_line_interrupt(IRQ_PIN, SX1278_PORT, RISING); /* EXTI Config */
}

void fsk_set_payload_len(uint16_t payload_len) {
	uint8_t reg_data;

	sx1278_read_reg(RegPacketConfig2, &reg_data);
	reg_data |= (uint8_t) (payload_len >> 9);
	sx1278_write_reg(RegPacketConfig2, reg_data);
	sx1278_write_reg(RegPayloadLength, (uint8_t) (payload_len >> 0));
}

static uint8_t wait_irq_flag(uint8_t flag_code) {
	uint8_t flag_reg = 0;
	uint32_t timeout = get_platform_tick_call();
	while ((flag_reg & flag_code) == 0 
			|| (get_platform_tick() - timeout) > 5000) {
		printf("irq_flags: %x\n", flag_reg);
		sx1278_read_reg(RegIrqFlags2, &flag_reg);
	}
	return 1;
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
static inline void fsk_kbps_fast(void) {
	/* Setting a kpbs of 80,
	 * fdev around 20KHz, 
	 * RxBw >= 200 
	 */
	uint16_t bitrate = KPBS_80;
	sx1278_write_reg(RegBitrateMsb, (uint8_t) (bitrate >> 8));
	sx1278_write_reg(RegBitrateLsb, (uint8_t) (bitrate >> 0));

	uint16_t fdev_reg = 0x148;
	sx1278_write_reg(RegFdevMsb, (uint8_t) (fdev_reg >> 8));
	sx1278_write_reg(RegFdevLsb, (uint8_t) (fdev_reg >> 0));

	uint8_t rxbw_reg = 0x9; 
	sx1278_write_reg(RegRxBw, rxbw_reg);

	//sx1278_write_reg(RegPreambleLsb, (uint8_t)(>preamb >> 8U));
}

/* h = 0.8 */
static inline void fsk_kbps_mid(void) {
	/* Setting a kpbs of 50,
	 * fdev around 20KHz, 
	 * RxBw >= 150 
	 */
	uint16_t bitrate = KPBS_50;
	sx1278_write_reg(RegBitrateMsb, (uint8_t) (bitrate >> 8));
	sx1278_write_reg(RegBitrateLsb, (uint8_t) (bitrate >> 0));

	uint16_t fdev_reg = 0x143;
	sx1278_write_reg(RegFdevMsb, (uint8_t) (fdev_reg >> 8));
	sx1278_write_reg(RegFdevLsb, (uint8_t) (fdev_reg >> 0));

	uint8_t rxbw_reg = 0x11; 
	sx1278_write_reg(RegRxBw, rxbw_reg);

	//sx1278_write_reg(RegPreambleLsb, (uint8_t)(>preamb >> 8U));
}

/* h = 1.0 */
static inline void fsk_kbps_slow(void) {
	/* Setting a kpbs of 10,
	 * fdev around 5KHz, 
	 * RxBw >= 30 
	 */
	uint16_t bitrate = KPBS_10;
	sx1278_write_reg(RegBitrateMsb, (uint8_t) (bitrate >> 8));
	sx1278_write_reg(RegBitrateLsb, (uint8_t) (bitrate >> 0));

	uint16_t fdev_reg = 0xA3;
	sx1278_write_reg(RegFdevMsb, (uint8_t) (fdev_reg >> 8));
	sx1278_write_reg(RegFdevLsb, (uint8_t) (fdev_reg >> 0));

	uint8_t rxbw_reg = 0x4; 
	sx1278_write_reg(RegRxBw, rxbw_reg);

	//sx1278_write_reg(RegPreambleLsb, (uint8_t)(>preamb >> 8U));
}

void sx1278_write_reg(uint8_t addr, uint8_t val) {
	uint8_t reg[2];
	static const size_t reg_len = 2;

	reg[0] = 0x80 | addr;
	reg[1] = val;

	platform_spi_call(chip_spi, reg, (uint8_t*)0, reg_len);
}

void sx1278_read_reg(uint8_t addr, uint8_t* out) {
	uint8_t reg[2];
	uint8_t rx_buf[2];
	static const size_t reg_len = 2;

	reg[0] = addr & 0x7F; 
	reg[1] = 0;
	platform_spi_call(chip_spi, reg, rx_buf, reg_len);
	
	*out = rx_buf[1];
}

void sx1278_burstwrite_fifo(uint8_t* payload, size_t payload_len) {
	/* This line will be kept as memorabilia, as it alone
	 * was the cause of a day of debugging why my payloads
	 * were not being received properly. 
	 */
	/* if (payload_len > 33) return; */
	size_t reg_len = payload_len + 1;
	uint8_t reg[reg_len];
	reg[0] = 0x80 | RegFifo;
	memcpy(&reg[2], payload, payload_len);

	platform_spi_call(chip_spi, reg, (uint8_t*)0, reg_len);
}

void sx1278_set_mode(uint8_t mode) {
	uint8_t curr_op = 0;

	sx1278_read_reg(RegOpMode, &curr_op);
	curr_op = (uint8_t) ((curr_op & ~7U) | mode); /* Overwrite mode bits */

	sx1278_write_reg(RegOpMode, curr_op);
	curr_mode = mode;
}
