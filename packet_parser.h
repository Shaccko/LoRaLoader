#include <stdint.h>

#define CHUNK_SIZE 200
#define SYNC_BYTE 0xAD
#define PKT_PASS 0xCF
#define PKT_FAIL 0xF

struct ota_pkt {
	uint8_t chunk_size, chunk_num, chunk_data[CHUNK_SIZE];
};

static inline uint8_t validate_packet_checksum(uint8_t* buf, size_t buf_size, uint8_t pkt_checksum) {
	uint8_t checksum = 0;

	size_t i;
	for (i = 0; i < buf_size; i++) {
		checksum ^= buf[i];	
	}
	if (checksum != pkt_checksum) return 0;

	return 1;
}

static inline uint8_t validate_packets_received(uint8_t* rx_pkt, struct ota_pkt* out_pkt) {
	/* [0] = header
	 * [1] = chunk_size
	 * [2] = chunk_num
	 * [3] = data[CHUNK_SIZE]
	 * [4] = checksum
	 */

	static uint8_t chunk_num = 1;

	if (rx_pkt[0] != SYNC_BYTE || rx_pkt[1] > 200 
			|| rx_pkt[2] != chunk_num) return PKT_FAIL;
	if (validate_packet_checksum(&rx_pkt[3], (size_t)rx_pkt[1],  rx_pkt[4]) != 1) return PKT_FAIL;
	
	out_pkt->chunk_size = rx_pkt[1];
	out_pkt->chunk_num = chunk_num++;
	memcpy(out_pkt->chunk_data, &rx_pkt[3], rx_pkt[1]);

	printf("pass\r\n");

	return PKT_PASS;
}
