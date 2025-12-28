#include <stdint.h>

#include <flash.h>
#include <packet_parser.h>
#include <uart.h>


void write_packet(uint8_t* rx_buf) {
	extern uint32_t _sflash_b;

	write_flash(rx_buf, &_sflash_b);
}

void kill_ota_firmware(void) {
	/* Kill firmware */
	ota_tx_rdy = 0;
	
	printf("Killing firmware\r\n");
	clear_flash_sectors(FLASHB_SECTOR);
}

