#include <stdint.h>
#include <string.h>

#include <flash.h>
#include <packet_parser.h>
#include <uart.h>
#include <sx1278_fsk.h>

static uint8_t ota_tx_rdy = 0;

void write_packet(uint8_t* rx_buf) {
	extern uint32_t _sflash_b;

	if (ota_tx_rdy == 0) ota_tx_rdy = 1;
	write_flash(rx_buf, &_sflash_b);
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
