#ifndef __PACKET_TRANSMITTER_H__
#define __PACKET_TRANSMITTER_H__

#include <stdint.h>
#include <signal.h>

#include <lora_raspi.h>
#include <gpio_raspi.h>

#define ACK_CODE 0xAC
#define OTA_BYTE 0xBC
#define OTA_TX_START 0xCC
#define PKT_PASS 0xEC
#define PKT_COMPLETE 0xAA
#define OTA_ERR 0xFF

#define CHUNK_SIZE 200

struct packet {
	uint8_t header, chunk_size, chunk_num, checksum, data[CHUNK_SIZE];
};

uint8_t send_tx_wait_ack(struct lora* lora, uint8_t* tx, size_t tx_len);
void generate_firmware_packet(struct packet* pkt, uint8_t* data_buf, size_t bytes_read);
void increment_chunk_num(void);

#endif
