#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include <gpio_raspi.h>
#include <spi_raspi.h>
#include <lora_raspi.h>

static uint8_t curr_mode = 0;

static inline uint8_t fifo_empty(void);
static void chip_reset();

uint8_t new_lora(struct lora* lora) {
	/* Default pins */
	lora->lora_port = LORA_PORT;
	lora->cs_pin = CS_PIN;
	lora->rst_pin = RST_PIN;
	lora->dio0_pin = IRQ_PIN;

	/* Set GPIO pins */
	gpio_raspi_set_mode(lora->rst_pin|lora->dio0_pin, PIN_OUTPUT);
	gpio_raspi_write_pin(lora->rst_pin|lora->dio0_pin, PIN_SET);

	/* Default values for loraWAN modem, don't care
	 * about messing with these.
	 * Should give us a range of 3-4km, according to lora
	 * modem calculate by semtech.
	 */
	lora->freq = FREQ_433;
	lora->sf = SF_7;
	lora->bw = BW_500KHz;
	lora->code_rate = CR_4_5;
	lora->preamb = PREAMB_8;
	lora->db_pwr = POWER_20dB;
	curr_mode = STDBY;

	/* Reset, flush FiFo */
	chip_reset();
	
	/* Initialize lora */
	set_mode(SLEEP);
	write_reg(RegOpMode, 0x80); /* Set RegOP to lora mode */

	/* Set frequency, output power gain, OCP, LNA gain, 
	 * SF, CRC, timeout MSB, timeout LSB, bandwidth, coding rate, explicit mode,
	 * preaamble, DI0 mapping then goto standby mode for further operations by user
	 * end with a version check for validity. 
	 */
	
	/* Essentially these registers below are increasing our range. */
	/* FRF = (F_rf * 2^19)/32MHz */
	set_freq(lora->freq);

	/* Power Gain */
	write_reg(RegGainConfig, lora->db_pwr);

	/* OCP */
	set_ocp();

	/* Set LNA Gain */
	set_lnahigh(); 

	/* Set BW, CR */
	set_modemconfig1(lora->bw, lora->code_rate);

	/* CF, CRC, Timeout */
	set_modemconfig2(lora->sf);

	/* DIO mapping, using DIO0 */
	uint8_t read, data;
	read_reg(RegDioMapping1, &read);
	data = read | 0x3FU; 
	write_reg(RegDioMapping1, data); /* Setting DIO0, rest to none */

	/* Set Preamble */
	write_reg(RegPreambleMsb, (uint8_t)(lora->preamb >> 8U));
	write_reg(RegPreambleLsb, (uint8_t)(lora->preamb >> 0U));

	/* Registers set, STDBY for future operations, check lora with version read */
	set_mode(STDBY);

	uint8_t lora_version;
	read_reg(RegVersion, &lora_version);

	/* We expect it to return 0x12, according to register datasheet */

	return lora_version == 0x12 ? OK : FAIL;
}

uint8_t new_fsk(struct fsk* fsk) {
	/* Default pins */
	fsk->fsk_port = LORA_PORT;
	fsk->cs_pin = CS_PIN;
	fsk->rst_pin = RST_PIN;
	fsk->dio_pin = IRQ_PIN;

	/* Set GPIO pins */
	gpio_raspi_set_mode(fsk->rst_pin|fsk->dio_pin, PIN_OUTPUT);
	gpio_raspi_write_pin(fsk->rst_pin|fsk->dio_pin, PIN_SET);

	fsk->bitrate = KPBS_300;
	fsk->preamb = PREAMB_8;
	fsk->db_pwr = POWER_20dB;
	fsk->curr_mode = STDBY;

	/* Reset, flush FiFo */
	chip_reset();
	
	/* Initialize fsk */
	set_mode(SLEEP);
	write_reg(RegOpMode, (1 << 5)); /* Set RegOP to fsk mode */

	/* Set birate */
	//fsk_set_bitrate(fsk->bitrate);


	/* Set Frequency Deviation */
	fsk_set_fdev(Fdev_50Khz);

	/* Leaving RF carrier freq at reset (434MHz) */

	/* Set PaRamp */
	write_reg(RegPaRamp, 0x2F); /* Gaussian filter = 1.0, 10us ramp */

	/* Power Gain */
	write_reg(RegGainConfig, fsk->db_pwr);

	/* RegRxBw */
	write_reg(RegRxBw, 0xB); /* 50KHz */

	/* OCP */
	set_ocp();

	/* Set LNA Gain */
	set_lnahigh(); 

	/* DIO mapping, using DIO0 */
	uint8_t read, data;
	read_reg(RegDioMapping1, &read);
	data = read | 0x3FU; 
	write_reg(RegDioMapping1, data); /* Setting DIO0, rest to none */

	/* Set Preamble */
	write_reg(RegPreambleMsb, (uint8_t)(fsk->preamb >> 8U));
	write_reg(RegPreambleLsb, (uint8_t)(fsk->preamb >> 0U));

	/* Registers set, STDBY for future operations, check fsk with version read */
	set_mode(STDBY);

	uint8_t lora_version;
	read_reg(RegVersion, &lora_version);

	/* We expect it to return 0x12, according to register datasheet */

	return lora_version == 0x12 ? OK : FAIL;
}

uint8_t fsk_transmit(uint8_t* msg, size_t msg_len) {
	uint8_t reg;
	uint8_t lora_mode = curr_mode;

	set_mode(STDBY);

	/* Set FiFo ptr to TxAddr ptr */
	lora_burstwrite(msg, msg_len); 
	set_mode(FSTX); /* Write to FiFo and Transmit */

	set_mode(lora_mode);

	return OK;
}

uint8_t lora_transmit(uint8_t* msg, size_t msg_len) {
	uint8_t reg;
	uint8_t lora_mode = curr_mode;

	set_mode(STDBY);
	write_reg(RegIrqFlags, 0xFFU); /* Pre-clear all flags */

	/* Set FiFo ptr to TxAddr ptr */
	read_reg(RegFifoTxBaseAddr, &reg);
	write_reg(RegFifoAddrPtr, reg);
	write_reg(FifoPayloadLength, (uint8_t)msg_len);

	lora_burstwrite(msg, msg_len); 
	set_mode(TX); /* Write to FiFo and Transmit */

	/* Check and clear Tx flag */
	do {
		read_reg(RegIrqFlags, &reg);
		usleep(1*1000);
	} while ((reg & 0x08U) == 0);

	write_reg(RegIrqFlags, 0xFFU); 
	set_mode(lora_mode);

	return OK;
}

uint8_t lora_receive(uint8_t* buf) {
	uint8_t addr;
	uint8_t num_bytes;

	set_mode(STDBY);

	/* Clear rx flag */
	write_reg(RegIrqFlags, 0xFFU);

	/* Set FiFo */
	read_reg(RegFifoRxCurrentAddr, &addr);
	write_reg(RegFifoAddrPtr, addr);
	read_reg(FifoRxBytesNb, &num_bytes);

	/* Read from FiFo, each consecutive read moves the FiFo
	 * address ptr up.
	 */
	for (uint8_t i = 0; i < num_bytes; i++) {
		read_reg(RegFifo, &buf[i]);
	}

	set_mode(RXCONT);

	return num_bytes;
}



static inline uint8_t fifo_empty(void) {
	uint8_t reg;

	/* Initial check if FiFo is empty */
	read_reg(RegOpMode, &reg);
	reg |= 0x40U;
	write_reg(RegOpMode, reg);
	read_reg(FSKIrqFlags2, &reg); /* Access FiFoEmpty */
	if (!reg) return FAIL;
	reg &= (uint8_t)(~(0x40U));
	write_reg(RegOpMode, reg); /* Reset back to lora registers */

	return OK;
}

void set_modemconfig2(uint8_t sf) {
	uint8_t reg_val;
	uint8_t read;

	read_reg(RegModemConfig2, &read);
	//reg_val = (read | ((uint8_t) (sf << 4U)) | 0x05U);
	reg_val = (read | ((uint8_t) (sf << 4U) | (0x07U)));
	write_reg(RegModemConfig2, reg_val);
	write_reg(RegSymbTimeoutLsb, 0xFFU); /* Set LSB TimeOut */
}

void set_modemconfig1(uint8_t bw, uint8_t code_rate) { 
	/* 4/5 code rate, 125KHz BW, implicit (no CRC) mode */
	uint8_t reg_val = (uint8_t) (((unsigned)bw << 4U) | ((unsigned)code_rate << 1U) | (0x00U)); /* Thank you C */
	write_reg(RegModemConfig1, reg_val);
}

void set_lnahigh(void) {
	uint8_t reg_val = 0x20 | 0x03; /* 150% LNA, G1 = max gain */
	write_reg(RegLNA, reg_val);
}

void set_ocp(void) {
	uint8_t reg_val = 0x20 | 0x0B; /* Sets OCP, default set to lmax = 100ma */
	write_reg(RegOCP, reg_val);
}

void set_freq(uint32_t freq) {
	uint8_t reg_data;
	uint32_t new_freq = ((freq * (1U << 19U)) >> 5U); /* freq * 2^19 / 2^5 */

	reg_data = (uint8_t) (new_freq >> 16U);
	write_reg(RegFrMsb, reg_data);

	reg_data = (uint8_t) (new_freq >> 8U);
	write_reg(RegFrMid, reg_data);

	reg_data = (uint8_t) (new_freq >> 0);
	write_reg(RegFrLsb, reg_data);
}

void fsk_set_bitrate(uint16_t bitrate) {
	uint8_t reg_data; 

	reg_data = (uint8_t) (bitrate >> 8);
	write_reg(RegBitrateMsb, reg_data);
	reg_data = (uint8_t) (bitrate >> 0);
	write_reg(RegBitrateLsb, reg_data);
}

void fsk_set_fdev(uint16_t fdev) {
	uint8_t reg_data;
	reg_data = (uint8_t) (fdev >> 8);
	write_reg(RegFdevMsb, reg_data);
	reg_data = (uint8_t) (fdev >> 0);
	write_reg(RegFdevLsb, reg_data);
}

void write_reg(uint8_t addr, uint8_t val) {
	uint8_t reg[2];
	static const size_t reg_len = 2;

	reg[0] = 0x80 | addr;
	reg[1] = val;

	spidev_transmit_receive(reg, (uint8_t*)0, reg_len);
}

void lora_burstwrite(uint8_t* payload, size_t payload_len) {
	/* This line will be kept as memorabilia, as it alone
	 * was the cause of a day of debugging why my payloads
	 * were not being received properly. 
	 */
	/* if (payload_len > 33) return; */

	uint8_t reg[payload_len];
	size_t reg_len = payload_len + 1;
	reg[0] = 0x80 | RegFifo;
	memcpy(&reg[1], payload, payload_len);

	spidev_transmit_receive(reg, (uint8_t*)0, reg_len);
}


void read_reg(uint8_t addr, uint8_t* out) {
	uint8_t reg[2];
	uint8_t rx_buf[2];
	static const size_t reg_len = 2;

	reg[0] = addr & 0x7F; 
	reg[1] = 0;

	spidev_transmit_receive(reg, rx_buf, reg_len);

	*out = rx_buf[1];
}

void set_mode(uint8_t mode) {
	uint8_t curr_op = 0;

	read_reg(RegOpMode, &curr_op);
	curr_op = (uint8_t) ((curr_op & ~7U) | mode); /* Overwrite mode bits */

	write_reg(RegOpMode, curr_op);
	curr_mode = mode;
}

static void chip_reset() {
	gpio_raspi_write_pin(RST_PIN, PIN_RESET);
	usleep(150);
	gpio_raspi_write_pin(RST_PIN, PIN_SET);
	usleep(10 * 1000);
}


/* FSK Notes:
 * LongRangeMode: RegOpMode
 * Modulation type: RegOpMode
 * Setting TX/RX modes in RegOpMode -> yeah
 * RegFrequency, MsbMidLsb -> 0x08
 * RegPaConfig -> 0x09
 * ModulationShaping -> 0x0A
 * RegPacketConfig -> 0x30
 * RegPacketConfig2 -> 0x31
 * RegFifoThresh -> 0x35
 * RegIrqFlags2 -> 0x3f
 */
