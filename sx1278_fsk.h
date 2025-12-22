#pragma once

#include <hal.h>

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

/* Packet Config Registers */
#define RegSyncConfig 0x27
#define RegPacketConfig1 0x30

/* FSK IRQ Registers */
#define FSKIrqFlags2 0x3F

/* Frequency Registers */
#define RegFrMsb 0x06
#define RegFrMid 0x07
#define RegFrLsb 0x08 /* 24 bits, divided among 3 registers */

/* Gain Power Register */
#define RegGainConfig 0x09

#define KPBS_50  (0x80 | 0x00)
#define KPBS_150 (0x00 | 0xD0)
#define KPBS_300 (0x00 | 0x6B)

/* OCP Register */
#define RegOCP 0x0B
/* LNA Register */
#define RegLNA 0x0C

/* Preamble Register */
#define RegPreambleMsb 0x20
#define RegPreambleLsb 0x21

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

#endif
