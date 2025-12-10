#include <stdint.h>
#include <lora_stm32.h>

#define CHUNK_SIZE 200
#define ACK_CODE 0xAC
#define OTA_BYTE 0xBC
#define OTA_TX_START 0xCC
#define OTA_TX_STOP 0xDC
#define PKT_PASS 0xEC
#define PKT_FAIL 0xF7

struct ota_pkt {
	uint8_t chunk_size, chunk_num, chunk_data[CHUNK_SIZE];
};

static inline uint8_t validate_packet_checksum(uint8_t* buf, size_t buf_size, uint8_t pkt_checksum) {
	uint8_t checksum = 0;

	size_t i;
	for (i = 0; i < buf_size; i++) {
		checksum ^= buf[i];	
	}
	printf("checksum: %x, pkt_checksum: %x\r\n", checksum, pkt_checksum);
	if (checksum != pkt_checksum) return checksum;

	return 1;
}

static inline uint8_t validate_packets_received(uint8_t* rx_pkt, struct ota_pkt* out_pkt) {
	/* [0] = header
	 * [1] = chunk_size
	 * [2] = chunk_num
	 * [3] = checksum
	 * [4-203] = data[CHUNK_SIZE]
	 */

	static uint8_t chunk_num = 1;

	if (rx_pkt[0] != OTA_BYTE || rx_pkt[1] > 200 
			|| rx_pkt[2] != chunk_num) {
		printf("failed1\r\n");
		return PKT_FAIL;
	}
	printf("Passed 1\r\n");
	if (validate_packet_checksum(&rx_pkt[4], (size_t)rx_pkt[1],  rx_pkt[3]) != 1) {
		printf("failed2\r\n");
		return PKT_FAIL;
	}
	printf("Passed 2\r\n");

	
	out_pkt->chunk_size = rx_pkt[1];
	out_pkt->chunk_num = chunk_num++;
	memcpy(out_pkt->chunk_data, &rx_pkt[3], rx_pkt[1]);


	return PKT_PASS;
}
