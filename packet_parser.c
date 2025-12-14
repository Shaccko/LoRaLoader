#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <flash.h>
#include <lora_stm32.h>
#include <packet_parser.h>

static uint8_t ota_tx_rdy = 0;

/* Chunk valid parse data */
static uint8_t chunk_num = 1;
//static uint32_t chunk_size = 0;

static inline uint8_t validate_packet_checksum(uint8_t* buf, size_t buf_size, uint8_t pkt_checksum) {
	uint8_t checksum = 0;

	size_t i;
	for (i = 0; i < buf_size; i++) {
		checksum ^= buf[i];	
	}
	if (checksum != pkt_checksum) return 0;

	return 1;
}

static uint8_t validate_packet_received(uint8_t* rx_buf) {
	if ((rx_buf[1] > CHUNK_SIZE || rx_buf[2] != chunk_num)) {
			printf("Failed packet size check\r\n");
			return PKT_FAIL;
	}
	if (validate_packet_checksum(&rx_buf[4], (size_t)rx_buf[1], rx_buf[3]) == 0) {
		printf("Failed checksum check\r\n");
		return PKT_FAIL;
	}
	printf("Passed checks\r\n");

	return PKT_PASS;
}

uint8_t parse_packet_state(uint8_t* rx_buf) {
	uint8_t pkt_state;
	switch (rx_buf[0]) {
		case (OTA_MAGIC_BYTE):
			ota_tx_rdy = 1;
			chunk_num = 1;
			pkt_state = ACK_CODE;
			break;
		case (OTA_PACKET_BYTE):
			printf("Received OTA packet\r\n");
			if (validate_packet_received(rx_buf) == PKT_PASS) {
				struct ota_pkt out_pkt;
				out_pkt.chunk_size = rx_buf[1];
				out_pkt.chunk_num = chunk_num++;
				memcpy(out_pkt.chunk_data, &rx_buf[3], CHUNK_SIZE);
				write_flash_b(&out_pkt);
			}
			pkt_state = PKT_PASS;
			break;
		case (PKT_COMPLETE):
			ota_tx_rdy = 0;
			chunk_num = 1;
			pkt_state = PKT_COMPLETE;
			break;
	}

	return pkt_state;
}

void kill_ota_firmware(void) {
	printf("Packet reception stalled, clearing sector data...\r\n");
	/* Kill firmware */
	ota_tx_rdy = 0;
	chunk_num = 1;

	clear_flash_sectors(FLASHB_SECTOR);
}


uint8_t get_ota_state(void) {
	return ota_tx_rdy;
}

void set_ota_state(void) {
	ota_tx_rdy = 1;
}
