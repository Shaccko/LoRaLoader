#include <stdint.h>

#include <flash.h>
#include <packet_parser.h>
#include <uart.h>

void write_packet(uint8_t* rx_buf) {
	extern uint32_t _sflash_b, _sflash_a;
	extern uint32_t _flash_ptr;
	uint32_t* write_addr;

	write_addr = ((uint32_t*)&_flash_ptr) == ((uint32_t*)&_sflash_a) ? 
		((uint32_t*)&_sflash_b) : ((uint32_t*)&_sflash_a);

	printf("write_addr: %lX\r\n", write_addr);
	if (get_ota_state() == 0) set_ota_state();
	switch (rx_buf[0]) {
		case OTA_PACKET_BYTE:
			write_flash(rx_buf, write_addr);
			break;
		case PKT_COMPLETE:
			set_flash_ptr(write_addr);
			clear_ota_state();
			printf("Firmware download complete, please restart MCU.\r\n");
			break;
	}

}


void kill_ota_firmware(void) {
	/* Kill firmware */
	ota_tx_rdy = 0;
	
	printf("Killing firmware\r\n");
	clear_flash_sectors(FLASHB_SECTOR);
}


/* boot pointer: old_flash and new_flash
 * at initial app flash, old_flash and new_flash point
 * to same address
 *
 * on firmware download, old_flash is unchanged,
 * new_flash is set to downloaded firmware address
 * if another incoming firmware detected, set old_flash
 * to new_flash, clear new_flash, set to incoming flash's
 * address
 *
 * initial routine:
 * old_flash = 0x08004000
 * new_flash = 0x08004000
 *
 * inc. flash
 * old_flash = 0x08004000
 * new_flash = 0x08040000
 *
 * okay maybe we can use these 2 as set 
 * static flash positions, and we can use a 
 * flash_ptr as a way to see which flash we are currently
 * at.
 *
 * flash_a = 0x08004000 <- flash_ptr
 * flash_b = 0x08040000
 *
 * incoming flash:
 *
 * check flash_ptr's position, download new flash in
 * flash region which flash_ptr is not pointing to
 *
 * flash_a = 0x08004000
 * flash_b = 0x08040000 <- flash_ptr
 *
 *
 * download into whatever region flash_ptr is NOT pointing at
 *
 *
 *
 *
 *
 *
 */

