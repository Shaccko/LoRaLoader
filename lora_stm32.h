#ifndef LORA_H__
#define LORA_H__

#include <hal.h>

/* FiFo buffer and OP modes */
#define RegFifo 0x00
#define RegOpMode 0x01

/* FiFo Addr Registers */
#define RegFifoAddrPtr 0x0D
#define RegFifoTxBaseAddr 0x0E
#define RegFifoRxBaseAddr 0x0F
#define RegFifoRxCurrentAddr 0x10
#define FifoRxBytesNb 0x13
#define FifoPayloadLength 0x22

/* Packet Config Registers */
#define RegSyncConfig 0x27
#define RegPacketConfig1 0x30

/* LoRa IRQ Registers */
#define RegIrqFlagsMask 0x11
#define RegIrqFlags 0x12

/* FSK IRQ Registers */
#define FSKIrqFlags2 0x3F


/* Frequency Registers */
#define RegFrMsb 0x06
#define RegFrMid 0x07
#define RegFrLsb 0x08 /* 24 bits, divided among 3 registers */

/* Gain Power Register */
#define RegGainConfig 0x09

/* OCP Register */
#define RegOCP 0x0B
/* LNA Register */
#define RegLNA 0x0C

/* BW, CR, and header mode config */
#define RegModemConfig1 0x1D
/* SF, CRC, and Timeout MSB */
#define RegModemConfig2 0x1E
/* LSB Timeout */
#define RegSymbTimeoutLsb 0x1F
/* LDRO config */
#define RegModemConfig3 0x26

/* Preamble Register */
#define RegPreambleMsb 0x20
#define RegPreambleLsb 0x21

/* DIOx Mapping Registers */
#define RegDioMapping1 0x40

/* LoRa version Register */
#define RegVersion 0x42

/* LoRa pins definitions */
#define LORA_PORT 'B'
#define CS_PIN (PIN_NUM(0))
#define RST_PIN (PIN_NUM(5))
#define IRQ_PIN (PIN_NUM(3))

/* LoRaWAN modem definitions */
#define FREQ_433 433
#define SF_7 0x07
#define BW_125KHz 0x07
#define BW_250KHz 0x08
#define CR_4_5 0x01
#define POWER_20dB 0xFF
#define OCP_100 0x100
#define PREAMB_8 0x08

/* T/F Macros */
#define OK 1
#define FAIL 0

enum { SLEEP, STDBY, FSTX, TX, FSRX, RXCONT, RXSINGLE, CAD };

struct lora {
	uint8_t lora_port;
	uint16_t cs_pin, rst_pin, dio0_pin;
	struct spi* lspi;

	uint32_t freq, ocp;
	uint8_t sf, bw, code_rate, preamb, db_pwr, curr_mode;
};

uint8_t new_lora(struct lora* lora);
void lora_write_reg(struct lora* lora, uint8_t addr, uint8_t val);
void lora_read_reg(struct lora* lora, uint8_t addr, uint8_t* out);
void lora_set_mode(struct lora* lora, uint8_t mode);
void lora_set_modemconfig2(struct lora* lora, uint8_t sf);
void lora_set_modemconfig1(struct lora* lora, uint8_t bw, uint8_t code_rate);
void lora_set_lnahigh(struct lora* lora);
void lora_set_ocp(struct lora* lora);
void lora_set_freq(struct lora* lora, uint32_t freq);
uint8_t lora_transmit(struct lora* lora, uint8_t* msg, size_t msg_len);
uint8_t lora_receive(struct lora* lora, uint8_t* buf);
void lora_burstwrite(struct lora* lora, uint8_t* payload, size_t payload_len);

#endif
