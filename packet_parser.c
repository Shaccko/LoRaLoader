#include <lora_stm32.h>
#include <packet_parser.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t ota_tx_rdy = 0;
static uint8_t chunk_num = 1;
static uint32_t chunk_size = 0;

static inline uint8_t validate_packet_checksum(uint8_t* buf, size_t buf_size, uint8_t pkt_checksum) {
	uint8_t checksum = 0;

	size_t i;
	for (i = 0; i < buf_size; i++) {
		checksum ^= buf[i];	
	}
	if (checksum != pkt_checksum) return checksum;

	return 1;
}

uint8_t validate_packets_received(uint8_t* rx_pkt, struct ota_pkt* out_pkt) {
	/* [0] = header
	 * [1] = chunk_size
	 * [2] = chunk_num
	 * [3] = checksum
	 * [4-203] = data[CHUNK_SIZE]
	 */
	switch (rx_pkt[0] & 0xFF) {
		case (PKT_COMPLETE):
			printf("case pkt_complete\r\n");
			ota_tx_rdy = 0;
			chunk_num = 1;
			printf("Chunk size: %d\r\n", chunk_size);
			return PKT_COMPLETE;
		case (OTA_TX_START):
			printf("case pkt_start\r\n");
			ota_tx_rdy = 1;
			chunk_num = 1;
			return ACK_CODE;
		case (OTA_BYTE):
			printf("case ota_byte\r\n");
			printf("rx_pkt[1]: %d, rx_pkt[2]: %d\r\n", rx_pkt[1], rx_pkt[2]);
			if ((rx_pkt[1] > 200) || rx_pkt[2] != chunk_num) {
				printf("Failed check 1\r\n");
				return PKT_FAIL;
			}

			printf("Passed 1\r\n");
			if (validate_packet_checksum(&rx_pkt[4], (size_t)rx_pkt[1],  rx_pkt[3]) != 1) {
				printf("failed2\r\n");
				return PKT_FAIL;
			}
			printf("Passed 2");

			out_pkt->chunk_size = rx_pkt[1];
			out_pkt->chunk_num = chunk_num++;
			memcpy(out_pkt->chunk_data, &rx_pkt[3], rx_pkt[1]);
			chunk_size = chunk_size + rx_pkt[1];	
			return PKT_PASS;
	}

	return PKT_FAIL;
}
void parse_packet_state(struct lora* lora, uint8_t* rx_pkt, struct ota_pkt* out_pkt) {
	printf("Received packet\r\n");
	uint8_t pkt_state = validate_packets_received(rx_pkt, out_pkt);
	/* if (pkt_state == PKT_COMPLETE)
	 * if (pkt_state == PKT_PASS)
	 * if (pkt_state == PKT_STOP);
	 */
	switch (pkt_state) {
		case (PKT_COMPLETE):
			/*move firmware flash*/
			break;
	}

	lora_transmit(lora, &pkt_state, 1);
}

void kill_ota_firmware(void) {
	printf("No incoming packets detected, erasing stored data.\r\n");
	/* Kill firmware */
	ota_tx_rdy = 0;
	chunk_num = 1;
}


uint8_t get_ota_state(void) {
	return ota_tx_rdy;
}

