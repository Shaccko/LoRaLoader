#pragma once

#include <hal.h>

/* Max FIFO Length */
#define FIFO_CHUNK 64

/* FiFo buffer and OP modes */
#define RegFifo 0x00
#define RegOpMode 0x01

/* FSK BitRate Regs */
#define RegBitrateMsb 0x02
#define RegBitrateLsb 0x03
/* kpbs values */
#define KPBS_10  (0x0D | 0x05)
#define KPBS_50  (0x80 | 0x00)
#define KPBS_80  (0x01 | 0xA1)
#define KBPS_100 (0x01 | 0x40)
#define KPBS_150 (0x00 | 0xD0)

/* Frequency Deviation Reg */
#define RegFdevMsb 0x04
#define RegFdevLsb 0x05

/* Receiver BW Reg */
#define RegRxBw 0x12

/* Packet Config Registers */
#define RegSyncConfig 0x27
#define RegPacketConfig1 0x30
#define RegPacketConfig2 0x31
#define RegPayloadLength 0x32

/* FSK IRQ Registers */
#define RegIrqFlags2 0x3F

/* Frequency Registers */
#define RegFrMsb 0x06
#define RegFrMid 0x07
#define RegFrLsb 0x08 /* 24 bits, divided among 3 registers */

/* Gain Power Register */
#define RegGainConfig 0x09

#define KPBS_50  (0x80 | 0x00)
#define KPBS_150 (0x00 | 0xD0)
#define KPBS_300 (0x00 | 0x6B)

/* Ramp Reg */
#define RegPaRamp 0x0A

/* OCP Register */
#define RegOCP 0x0B
/* LNA Register */
#define RegLNA 0x0C

/* Preamble Register */
#define RegPreambleMsb 0x20
#define RegPreambleLsb 0x21

/* RegFifoThresh reg */
#define RegFifoThresh 0x35

/* DIOx Mapping Registers */
#define RegDioMapping1 0x40

/* Modem version Register */
#define RegVersion 0x42

/* Chip pin definitions */
#define SX1278_PORT 'B'
#define CS_PIN (PIN_NUM(0))
#define RST_PIN (PIN_NUM(5))
#define IRQ_PIN (PIN_NUM(3))

/* Modem definitions */
#define POWER_20dB 0xFF
#define OCP_100 0x100
#define PREAMB_8 0x08

/* T/F Macros */
#define OK 1
#define FAIL 0

enum { SLEEP, STDBY, FSTX, TX, FSRX, RXCONT, RXSINGLE, CAD };

enum {
	LOW_BAT = 0x01,
	CRC_OK = 0x02,
	PAYLOAD_READY = 0x04,
	PACKET_SENT = 0x08,
	FIFO_OVERRUN = 0x10,
	FIFO_LEVEL = 0x20,
	FIFO_EMPTY = 0x40,
	FIFO_FULL = 0x80
};

uint8_t init_fsk(void);
uint8_t fsk_transmit_stream(uint8_t* msg, size_t msg_len);
uint8_t fsk_transmit(uint8_t* msg, size_t msg_len);

void sx1278_write_reg(uint8_t addr, uint8_t val);
void sx1278_read_reg(uint8_t addr, uint8_t* out);
void sx1278_burstwrite_fifo(uint8_t* payload, size_t payload_len);
void sx1278_set_mode(uint8_t mode);



