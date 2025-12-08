#include <stdint.h>

#define CHUNK_SIZE 200
#define ACK_CODE 0xCF
#define PACKET_FAIL_CODE 0xF

static inline void process_firmware_packet(uint8_t* firmware_info, uint8_t* rx_buf) {
	/* firmware_info[0] holds checksum, [1] holds total
	 * chunks transmitted 
	 */
	size_t i;
	for (i = 0; i < CHUNK_SIZE; i++) {
		firmware_info[0] ^= rx_buf[i];
	}
	firmware_info[1]++;
}

static inline uint8_t validate_packets_received(uint8_t* firmware_info, uint8_t* rx_buf) {
	uint8_t tx_checksum = rx_buf[0];
	uint8_t tx_total_chunks = rx_buf[1];

	printf("%x, %d\r\n", firmware_info[0], firmware_info[1]);
	if (firmware_info[0] == tx_checksum && 
	    firmware_info[1] == tx_total_chunks) {
		return ACK_CODE;
	}

	return PACKET_FAIL_CODE;
}
