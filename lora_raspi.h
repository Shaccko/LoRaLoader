#ifndef LORA_H__
#define LORA_H__

#include <stdint.h>
#include <stddef.h>

/* FiFo buffer and OP modes */
#define RegFifo 0x00
#define RegOpMode 0x01

/* FSK BitRate Regs */
#define RegBitrateMsb 0x02
#define RegBitrateLsb 0x03
/* kpbs */
#define KPBS_50  (0x80 | 0x00)
#define KPBS_150 (0x00 | 0xD5)
#define KPBS_300 (0x00 | 0x6B)

/* FSK PaRamp Reg */
#define RegPaRamp 0x0A
#define us_10 0x10

/* RegRxBw */
#define RegRxBw 0x12

/* FSK Frequency Deviation Reg */
#define RegFdevMsb 0x04
#define RegFdevLsb 0x05
#define Fdev_50Khz 0x19A

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

/* Modulation shape */
#define ModulationShaping 0x0A

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

/* RegPacketConfig */
#define RegPacketConfig 0x30
#define RegPacketConfig2 0x31

/* RegFifoThresh */
#define RegFifoThresh 0x35

/* FSK RegIrqFlags2 */
#define RegIrqFlags2 0x3F

/* DIOx Mapping Registers */
#define RegDioMapping1 0x40

/* LoRa version Register */
#define RegVersion 0x42

/* LoRa pins definitions */
#define LORA_PORT 'B'
/* Reconfig for raspi */
#define CS_PIN (PIN_NUM(27)) /* Hardware CS is used instead */
#define RST_PIN (PIN_NUM(22))
#define IRQ_PIN (PIN_NUM(23))

/* LoRaWAN modem definitions */
#define FREQ_433 433
#define SF_7 0x07
#define BW_125KHz 0x07
#define BW_250KHz 0x08
#define BW_500KHz 0x09
#define CR_4_5 0x01
#define POWER_20dB 0xFF
#define OCP_100 0x100
#define PREAMB_8 0x08

/* T/F Macros */
#define OK 1
#define FAIL 0

/* Helper */
#define PIN_NUM(x) (1U << (x))

enum { SLEEP, STDBY, FSTX, TX, FSRX, RXCONT, RXSINGLE, CAD };

struct lora {
	uint8_t lora_port;
	uint32_t cs_pin, rst_pin, dio0_pin;
	struct spi* lspi;

	uint32_t freq, ocp;
	uint8_t sf, bw, code_rate, preamb, db_pwr, curr_mode;
};

struct fsk {
	uint8_t fsk_port;
	uint32_t cs_pin, rst_pin, dio_pin;
	struct spi* lspi;

	uint16_t bitrate;
	uint8_t ocp, preamb, db_pwr, curr_mode;
};

uint8_t new_lora(struct lora* lora);
uint8_t new_fsk(struct fsk* fsk);
void set_mode(uint8_t mode);
void set_modemconfig2(uint8_t sf);
void set_modemconfig1(uint8_t bw, uint8_t code_rate);
void set_lnahigh(void);
void set_ocp(void);
void set_freq(uint32_t freq);
void fsk_set_bitrate(uint16_t bitrate);
void fsk_set_fdev(uint16_t fdev);
uint8_t lora_transmit(uint8_t* msg, size_t msg_len);
uint8_t lora_receive(uint8_t* buf);
void lora_burstwrite(uint8_t* payload, size_t payload_len);
void write_reg(uint8_t addr, uint8_t val);
void read_reg(uint8_t addr, uint8_t* out);

#endif
