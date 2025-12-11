#include <lora_stm32.h>
#include <hal.h>
#include <spi_stm32.h>
#include <uart.h>
#include <exti.h>

#include <string.h>


static inline uint8_t fifo_empty(struct lora* lora);


uint8_t new_lora(struct lora* lora) {
	uint8_t lora_version;
	/* Default pins */
	lora->lora_port = LORA_PORT;
	lora->cs_pin = CS_PIN;
	lora->rst_pin = RST_PIN;
	lora->dio0_pin = IRQ_PIN;
	lora->lspi = spi1;

	gpio_set_mode(CS_PIN|RST_PIN, GPIO_MODE_OUTPUT, LORA_PORT);
	gpio_set_mode(IRQ_PIN, GPIO_MODE_INPUT, LORA_PORT);
	gpio_set_pupdr(IRQ_PIN, PULL_DOWN, LORA_PORT);
	enable_line_interrupt(IRQ_PIN, LORA_PORT, RISING); 
	/* Set CS pin */ 
	gpio_write_pin(LORA_PORT, CS_PIN|RST_PIN, GPIO_PIN_SET); 

	/* Default values for loraWAN modem, don't care
	 * about messing with these.
	 * Should give us a range of 3-4km, according to lora
	 * modem calculate by semtech.
	 */
	lora->freq = FREQ_433;
	lora->sf = SF_7;
	//lora->bw = BW_125KHz;
	lora->bw = BW_250KHz;
	lora->code_rate = CR_4_5;
	lora->preamb = PREAMB_8;
	lora->db_pwr = POWER_20dB;
	lora->curr_mode = STDBY;

	/* Initialize lora */
	lora_set_mode(lora, SLEEP);
	lora_write_reg(lora, RegOpMode, 0x80); /* Set RegOP to lora mode */
	
	/* Set frequency, output power gain, OCP, LNA gain, 
	 * SF, CRC, timeout MSB, timeout LSB, bandwidth, coding rate, explicit mode,
	 * preaamble, DI0 mapping then goto standby mode for further operations by user
	 * end with a version check for validity. 
	 */
	
	/* Essentially these registers below are increasing our range. */
	/* FRF = (F_rf * 2^19)/32MHz */
	lora_set_freq(lora, lora->freq);

	/* Power Gain */
	lora_write_reg(lora, RegGainConfig, lora->db_pwr);

	/* OCP */
	lora_set_ocp(lora);

	/* Set LNA Gain */
	lora_set_lnahigh(lora); 

	/* Set BW, CR */
	lora_set_modemconfig1(lora, lora->bw, lora->code_rate);

	/* CF, CRC, Timeout */
	lora_set_modemconfig2(lora, lora->sf);

	/* DIO mapping, using DIO0 */
	uint8_t read, data;
	lora_read_reg(lora, RegDioMapping1, &read);
	data = read | 0x3FU; 
	lora_write_reg(lora, RegDioMapping1, data); /* Setting DIO0, rest to none */

	/* Set Preamble */
	lora_write_reg(lora, RegPreambleMsb, (uint8_t)(lora->preamb >> 8U));
	lora_write_reg(lora, RegPreambleLsb, (uint8_t)(lora->preamb >> 0U));

	/* Registers set, STDBY for future operations, check lora with version read */
	lora_set_mode(lora, STDBY);
	delay(10);

	lora_read_reg(lora, RegVersion, &lora_version);

	/* We expect it to return 0x12, according to register datasheet */
	return lora_version == 0x12 ? OK : FAIL;
}

uint8_t lora_transmit(struct lora* lora, uint8_t* msg, size_t msg_len) {

	uint8_t reg;
	uint8_t lora_mode = lora->curr_mode;

	lora_set_mode(lora, STDBY);
	lora_write_reg(lora, RegIrqFlags, 0xFFU); /* Pre-clear all flags */

	/* Set FiFo ptr to TxAddr ptr */
	lora_read_reg(lora, RegFifoTxBaseAddr, &reg);
	lora_write_reg(lora, RegFifoAddrPtr, reg);
	lora_write_reg(lora, FifoPayloadLength, (uint8_t)msg_len);

	lora_burstwrite(lora, msg, msg_len); 
	lora_set_mode(lora, TX); /* Write to FiFo and Transmit */

	/* Check and clear Tx flag */
	lora_write_reg(lora, RegIrqFlags, 0xFFU); /* Write 1 to clear flag */
	lora_set_mode(lora, lora_mode);

	return OK;
}

uint8_t lora_receive(struct lora* lora, uint8_t* buf) {
	uint8_t addr;
	uint8_t num_bytes;

	lora_set_mode(lora, STDBY);

	/* Clear rx flag */
	lora_write_reg(lora, RegIrqFlags, 0xFFU);

	/* Set FiFo */
	lora_read_reg(lora, RegFifoRxCurrentAddr, &addr);
	lora_write_reg(lora, RegFifoAddrPtr, addr);
	lora_read_reg(lora, FifoRxBytesNb, &num_bytes);

	/* Read from FiFo, each consecutive read moves the FiFo
	 * address ptr up.
	 */
	for (uint8_t i = 0; i < num_bytes; i++) {
		lora_read_reg(lora, RegFifo, &buf[i]);
	}

	lora_set_mode(lora, RXCONT);

	printf("Bytes: %d\r\n", num_bytes);
	return num_bytes;
}

static inline uint8_t fifo_empty(struct lora* lora) {
	uint8_t reg;

	/* Initial check if FiFo is empty */
	lora_read_reg(lora, RegOpMode, &reg);
	reg |= 0x40U;
	lora_write_reg(lora, RegOpMode, reg);
	lora_read_reg(lora, FSKIrqFlags2, &reg); /* Access FiFoEmpty */
	if (!reg) return FAIL;
	reg &= (uint8_t)(~(0x40U));
	lora_write_reg(lora, RegOpMode, reg); /* Reset back to lora registers */

	return OK;
}


void lora_set_modemconfig2(struct lora* lora, uint8_t sf) {
	uint8_t reg_val;
	uint8_t read;

	lora_read_reg(lora, RegModemConfig2, &read);
	//reg_val = (read | ((uint8_t) (sf << 4U)) | 0x05U);
	reg_val = (read | ((uint8_t) (sf << 4U)) | (0x07U));
	lora_write_reg(lora, RegModemConfig2, reg_val);
	lora_write_reg(lora, RegSymbTimeoutLsb, 0xFFU); /* Set LSB TimeOut */
}

void lora_set_modemconfig1(struct lora* lora, uint8_t bw, uint8_t code_rate) { 
	/* 4/5 code rate, 125KHz BW, explicit mode */
	uint8_t reg_val = (uint8_t) (((unsigned)bw << 4U) | ((unsigned)code_rate << 1U) | (0x00U)); /* Thank you C */
	lora_write_reg(lora, RegModemConfig1, reg_val);
}

void lora_set_lnahigh(struct lora* lora) {
	uint8_t reg_val = 0x20 | 0x03; /* 150% LNA, G1 = max gain */
	lora_write_reg(lora, RegLNA, reg_val);
}

void lora_set_ocp(struct lora* lora) {
	uint8_t reg_val = 0x20 | 0x0B; /* Sets OCP, default set to lmax = 100ma */
	lora_write_reg(lora, RegOCP, reg_val);
}

void lora_set_freq(struct lora* lora, uint32_t freq) {
	uint8_t reg_data;
	uint32_t new_freq = ((freq * (1U << 19U)) >> 5U); /* freq * 2^19 / 2^5 */

	reg_data = (uint8_t) (new_freq >> 16U);
	lora_write_reg(lora, RegFrMsb, reg_data);

	reg_data = (uint8_t) (new_freq >> 8U);
	lora_write_reg(lora, RegFrMid, reg_data);

	reg_data = (uint8_t) (new_freq >> 0);
	lora_write_reg(lora, RegFrLsb, reg_data);
}


void lora_write_reg(struct lora* lora, uint8_t addr, uint8_t val) {
	uint8_t reg[2];
	static const size_t reg_len = 2;

	reg[0] = 0x80 | addr;
	reg[1] = val;

	gpio_write_pin(LORA_PORT, CS_PIN, GPIO_PIN_RESET);
	spi_transmit_receive(lora->lspi, reg, 0, reg_len);
	gpio_write_pin(LORA_PORT, CS_PIN, GPIO_PIN_SET);
}

void lora_burstwrite(struct lora* lora, uint8_t* payload, size_t payload_len) {
	/* This line will be kept as memorabilia, as it alone
	 * was the cause of a day of debugging why my payloads
	 * were not being received properly. 
	 */
	/* if (payload_len > 33) return; */

	uint8_t reg[32];
	size_t reg_len = payload_len + 1;
	reg[0] = 0x80 | RegFifo;
	memcpy(&reg[1], payload, payload_len);

	gpio_write_pin(lora->lora_port, lora->cs_pin, GPIO_PIN_RESET);
	spi_transmit_receive(lora->lspi, reg, (uint8_t*)0, reg_len);
	gpio_write_pin(lora->lora_port, lora->cs_pin, GPIO_PIN_SET);
}


void lora_read_reg(struct lora* lora, uint8_t addr, uint8_t* out) {
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

void lora_set_mode(struct lora* lora, uint8_t mode) {
	uint8_t curr_op = 0;

	lora_read_reg(lora, RegOpMode, &curr_op);
	curr_op = (uint8_t) ((curr_op & ~7U) | mode); /* Overwrite mode bits */

	lora_write_reg(lora, RegOpMode, curr_op);
	lora->curr_mode = mode;
}
