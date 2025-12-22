#ifndef __PACKET_PARSER_H__
#define __PACKET_PARSER_H__

#include <stdint.h>

#define CHUNK_SIZE 200
#define CHECKSUM_CODE 0x3
#define ACK_CODE 0xAC
#define OTA_PACKET_BYTE 0xBC
#define OTA_MAGIC_BYTE 0xCC
#define PKT_PASS 0xEC
#define PKT_FAIL 0xF7
#define PKT_COMPLETE 0xAA

uint8_t parse_packet_state(uint8_t* rx_buf);
void kill_ota_firmware(void);
uint8_t get_ota_state(void);
void set_ota_state(void);

#endif 
