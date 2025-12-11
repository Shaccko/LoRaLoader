#ifndef __PACKET_PARSER_H__
#define __PACKET_PARSER_H__

#include <lora_stm32.h>
#include <stdint.h>

#define CHUNK_SIZE 200
#define ACK_CODE 0xAC
#define OTA_BYTE 0xBC
#define OTA_TX_START 0xCC
#define PKT_PASS 0xEC
#define PKT_FAIL 0xF7
#define PKT_COMPLETE 0xAA


struct ota_pkt {
	uint8_t chunk_size, chunk_num, chunk_data[CHUNK_SIZE];
};

void kill_ota_firmware(void);
uint8_t validate_packets_received(uint8_t* rx_pkt, struct ota_pkt* out_pkt);
uint8_t get_ota_state(void);
void parse_packet_state(struct lora* lora, uint8_t* rx_pkt, struct ota_pkt* out_pkt);

#endif 
