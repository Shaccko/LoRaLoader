#include <lora_raspi.h>
#include <gpio_raspi.h>
#include <spi_raspi.h>

#include <string.h>
#include <stddef.h>
#include <unistd.h>


static inline uint8_t fifo_empty(void);
static void lora_reset(struct lora* lora);

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
	lora->bw = BW_125KHz;
	lora->code_rate = CR_4_5;
	lora->preamb = PREAMB_8;
	lora->db_pwr = POWER_20dB;
	lora->curr_mode = STDBY;

	uint8_t irq;
	printf("Before setting params:\r\n");
	printf("RxFifo initial addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoRxBaseAddr, &irq);
	printf("RxBaseAddr: %x\r\n", irq);
	lora_read_reg(RegFifoRxCurrentAddr, &irq);
	printf("RxCurrAddr: %x\r\n", irq);
	lora_read_reg(FifoRxBytesNb, &irq);
	printf("RxBytesNb: %x\r\n", irq);

	printf("TxFifo initial addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoTxBaseAddr, &irq);
	printf("TxBaseAddr: %x\r\n", irq);
	lora_read_reg(FifoPayloadLength, &irq);
	printf("PayloadLengthTx: %x\r\n", irq);

	/* Reset, flush FiFo */
	lora_reset(lora);
	
	lora_read_reg(RegOpMode, &irq);
	/* Initialize lora */
	lora_set_mode(lora, SLEEP);
	usleep(10*1000);
	lora_write_reg(RegOpMode, 0x80); /* Set RegOP to lora mode */

	lora_read_reg(RegOpMode, &irq);

	/* Set frequency, output power gain, OCP, LNA gain, 
	 * SF, CRC, timeout MSB, timeout LSB, bandwidth, coding rate, explicit mode,
	 * preaamble, DI0 mapping then goto standby mode for further operations by user
	 * end with a version check for validity. 
	 */
	
	/* Essentially these registers below are increasing our range. */
	/* FRF = (F_rf * 2^19)/32MHz */
	lora_set_freq(lora->freq);

	/* Power Gain */
	lora_write_reg(RegGainConfig, lora->db_pwr);

	/* OCP */
	lora_set_ocp();

	/* Set LNA Gain */
	lora_set_lnahigh(); 

	/* Set BW, CR */
	lora_set_modemconfig1(lora->bw, lora->code_rate);

	/* CF, CRC, Timeout */
	lora_set_modemconfig2(lora->sf);

	/* DIO mapping, using DIO0 */
	uint8_t read, data;
	lora_read_reg(RegDioMapping1, &read);
	data = read | 0x3FU; 
	lora_write_reg(RegDioMapping1, data); /* Setting DIO0, rest to none */

	/* Set Preamble */
	lora_write_reg(RegPreambleMsb, (uint8_t)(lora->preamb >> 8U));
	lora_write_reg(RegPreambleLsb, (uint8_t)(lora->preamb >> 0U));

	/* Registers set, STDBY for future operations, check lora with version read */
	lora_set_mode(lora, STDBY);
	usleep(10*1000);
	uint8_t lora_version;
	lora_read_reg(RegVersion, &lora_version);

	printf("After setting params:\r\n");
	printf("RxFifo initial addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoRxBaseAddr, &irq);
	printf("RxBaseAddr: %x\r\n", irq);
	lora_read_reg(RegFifoRxCurrentAddr, &irq);
	printf("RxCurrAddr: %x\r\n", irq);
	lora_read_reg(FifoRxBytesNb, &irq);
	printf("RxBytesNb: %x\r\n", irq);

	printf("TxFifo initial addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoTxBaseAddr, &irq);
	printf("TxBaseAddr: %x\r\n", irq);
	lora_read_reg(FifoPayloadLength, &irq);
	printf("PayloadLengthTx: %x\r\n", irq);
	
	/* We expect it to return 0x12, according to register datasheet */

	return lora_version == 0x12 ? OK : FAIL;
}

uint8_t lora_transmit(struct lora* lora, uint8_t* msg, size_t msg_len) {

	uint8_t reg, irq;
	uint8_t lora_mode = lora->curr_mode;

	printf("On fresh transmit:\n");
	printf("RxFifo initial addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoRxBaseAddr, &irq);
	printf("RxBaseAddr: %x\r\n", irq);
	lora_read_reg(RegFifoRxCurrentAddr, &irq);
	printf("RxCurrAddr: %x\r\n", irq);
	lora_read_reg(FifoRxBytesNb, &irq);
	printf("RxBytesNb: %x\r\n", irq);

	printf("TxFifo initial addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoTxBaseAddr, &irq);
	printf("TxBaseAddr: %x\r\n", irq);
	lora_read_reg(FifoPayloadLength, &irq);
	printf("PayloadLengthTx: %x\r\n", irq);

	lora_set_mode(lora, STDBY);
	lora_write_reg(RegIrqFlags, 0xFFU); /* Pre-clear all flags */

	/* Set FiFo ptr to TxAddr ptr */
	lora_read_reg(RegFifoTxBaseAddr, &reg);
	lora_write_reg(RegFifoAddrPtr, reg);
	lora_write_reg(FifoPayloadLength, (uint8_t)msg_len);

	printf("After setting fifo ptrs:\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoTxBaseAddr, &irq);
	printf("TxBaseAddr: %x\r\n", irq);
	lora_read_reg(FifoPayloadLength, &irq);
	printf("PayloadLengthTx: %x\r\n", irq);
	printf("\n\n\n");

	lora_burstwrite(msg, msg_len); 
	lora_set_mode(lora, TX); /* Write to FiFo and Transmit */

	/* Check and clear Tx flag */
	do {
		lora_read_reg(RegIrqFlags, &reg);
		usleep(1*1000);
	} while ((reg & 0x08U) == 0);

	lora_write_reg(RegIrqFlags, 0xFFU); 
	lora_set_mode(lora, lora_mode);

	printf("On transmit done:\n");
	printf("RxFifo addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoRxBaseAddr, &irq);
	printf("RxBaseAddr: %x\r\n", irq);
	lora_read_reg(RegFifoRxCurrentAddr, &irq);
	printf("RxCurrAddr: %x\r\n", irq);
	lora_read_reg(FifoRxBytesNb, &irq);
	printf("RxBytesNb: %x\r\n", irq);

	printf("TxFifo addresses:\r\n");
	lora_read_reg(RegFifoAddrPtr, &irq);
	printf("FifoAddrPtr: %x\r\n", irq);
	lora_read_reg(RegFifoTxBaseAddr, &irq);
	printf("TxBaseAddr: %x\r\n", irq);
	lora_read_reg(FifoPayloadLength, &irq);
	printf("PayloadLengthTx: %x\r\n", irq);

	return OK;
}

uint8_t lora_receive(struct lora* lora, uint8_t* buf) {
	uint8_t addr;
	uint8_t num_bytes;

	lora_set_mode(lora, STDBY);

	/* Clear rx flag */
	lora_write_reg(RegIrqFlags, 0xFFU);

	/* Set FiFo */
	lora_read_reg(RegFifoRxCurrentAddr, &addr);
	lora_write_reg(RegFifoAddrPtr, addr);
	lora_read_reg(FifoRxBytesNb, &num_bytes);

	/* Read from FiFo, each consecutive read moves the FiFo
	 * address ptr up.
	 */
	for (uint8_t i = 0; i < num_bytes; i++) {
		lora_read_reg(RegFifo, &buf[i]);
	}

	lora_set_mode(lora, RXCONT);

	printf("Bytes: %d\n", num_bytes);
	return num_bytes;
}



static inline uint8_t fifo_empty(void) {
	uint8_t reg;

	/* Initial check if FiFo is empty */
	lora_read_reg(RegOpMode, &reg);
	reg |= 0x40U;
	lora_write_reg(RegOpMode, reg);
	lora_read_reg(FSKIrqFlags2, &reg); /* Access FiFoEmpty */
	if (!reg) return FAIL;
	reg &= (uint8_t)(~(0x40U));
	lora_write_reg(RegOpMode, reg); /* Reset back to lora registers */

	return OK;
}

void lora_set_modemconfig2(uint8_t sf) {
	uint8_t reg_val;
	uint8_t read;

	lora_read_reg(RegModemConfig2, &read);
	//reg_val = (read | ((uint8_t) (sf << 4U)) | 0x05U);
	reg_val = (read | ((uint8_t) (sf << 4U) | (0x07U)));
	lora_write_reg(RegModemConfig2, reg_val);
	lora_write_reg(RegSymbTimeoutLsb, 0xFFU); /* Set LSB TimeOut */
}

void lora_set_modemconfig1(uint8_t bw, uint8_t code_rate) { 
	/* 4/5 code rate, 125KHz BW, implicit (no CRC) mode */
	uint8_t reg_val = (uint8_t) (((unsigned)bw << 4U) | ((unsigned)code_rate << 1U) | (0x00U)); /* Thank you C */
	lora_write_reg(RegModemConfig1, reg_val);
}

void lora_set_lnahigh(void) {
	uint8_t reg_val = 0x20 | 0x03; /* 150% LNA, G1 = max gain */
	lora_write_reg(RegLNA, reg_val);
}

void lora_set_ocp(void) {
	uint8_t reg_val = 0x20 | 0x0B; /* Sets OCP, default set to lmax = 100ma */
	lora_write_reg(RegOCP, reg_val);
}

void lora_set_freq(uint32_t freq) {
	uint8_t reg_data;
	uint32_t new_freq = ((freq * (1U << 19U)) >> 5U); /* freq * 2^19 / 2^5 */

	reg_data = (uint8_t) (new_freq >> 16U);
	lora_write_reg(RegFrMsb, reg_data);

	reg_data = (uint8_t) (new_freq >> 8U);
	lora_write_reg(RegFrMid, reg_data);

	reg_data = (uint8_t) (new_freq >> 0);
	lora_write_reg(RegFrLsb, reg_data);
}


void lora_write_reg(uint8_t addr, uint8_t val) {
	uint8_t reg[2];
	static const size_t reg_len = 2;

	reg[0] = 0x80 | addr;
	reg[1] = val;

	spidev_transmit_receive(reg, (uint8_t*)0, reg_len);
}

void lora_burstwrite(uint8_t* payload, size_t payload_len) {
	if (payload_len > 33) return;

	uint8_t reg[32];
	size_t reg_len = payload_len + 1;
	reg[0] = 0x80 | RegFifo;
	memcpy(&reg[1], payload, payload_len);

	spidev_transmit_receive(reg, (uint8_t*)0, reg_len);
}


void lora_read_reg(uint8_t addr, uint8_t* out) {
	uint8_t reg[2];
	uint8_t rx_buf[2];
	static const size_t reg_len = 2;

	reg[0] = addr & 0x7F; 
	reg[1] = 0;

	spidev_transmit_receive(reg, rx_buf, reg_len);

	*out = rx_buf[1];
}

void lora_set_mode(struct lora* lora, uint8_t mode) {
	uint8_t curr_op = 0;

	lora_read_reg(RegOpMode, &curr_op);
	curr_op = (uint8_t) ((curr_op & ~7U) | mode); /* Overwrite mode bits */

	lora_write_reg(RegOpMode, curr_op);
	lora->curr_mode = mode;
}

static void lora_reset(struct lora* lora) {
	gpio_raspi_write_pin(lora->rst_pin, PIN_RESET);
	usleep(150);
	gpio_raspi_write_pin(lora->rst_pin, PIN_SET);
	usleep(10 * 1000);
}
