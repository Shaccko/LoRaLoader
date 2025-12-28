#ifndef __PACKET_PARSER_H__
#define __PACKET_PARSER_H__

#include <stdint.h>

#define CHUNK_SIZE 63 

#define CHECKSUM_CODE 0x3
#define ACK_CODE 0xAC
#define OTA_PACKET_BYTE 0xBC
#define OTA_MAGIC_BYTE 0xCC
#define PKT_PASS 0xEC
#define PKT_FAIL 0xF7
#define PKT_COMPLETE 0xAA

static uint8_t ota_tx_rdy = 0;

void write_packet(uint8_t* rx_buf);
void kill_ota_firmware(void);

static inline void set_ota_state(void) {
	ota_tx_rdy = 1;
}

static inline void clear_ota_state(void) {
	ota_tx_rdy = 0;
}

static inline uint8_t get_ota_state(void) {
	return ota_tx_rdy;
}
#endif 
