#include <stdint.h>
#include <string.h>

#include <flash.h>
#include <lora_stm32.h>
#include <packet_parser.h>
#include <uart.h>

static uint8_t ota_tx_rdy = 0;

uint8_t parse_packet_state(uint8_t* rx_buf) {
	uint8_t pkt_state;
	uint8_t bin_data[CHUNK_SIZE];
	switch (rx_buf[0]) {
		case (OTA_MAGIC_BYTE):
			ota_tx_rdy = 1;
			pkt_state = ACK_CODE;
			break;
		case (OTA_PACKET_BYTE):
			memcpy(bin_data, &rx_buf[1], CHUNK_SIZE);
			uart_write_buf(uart2, "Writing to Flash B\r\n", 20);
			write_flash_b(bin_data);
			pkt_state = PKT_PASS;
			break;
		case (PKT_COMPLETE):
			ota_tx_rdy = 0;
			pkt_state = PKT_COMPLETE;
			break;
	}

	return pkt_state;
}

void kill_ota_firmware(void) {
	/* Kill firmware */
	ota_tx_rdy = 0;

	clear_flash_sectors(FLASHB_SECTOR);
}


uint8_t get_ota_state(void) {
	return ota_tx_rdy;
}

void set_ota_state(void) {
	ota_tx_rdy = 1;
}
