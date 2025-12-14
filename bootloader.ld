ENTRY(__bootloader_reset);
/* Total flash: 512k
 * Total sram: 128k
 */
MEMORY {
	bootloader (rx) : ORIGIN = 0x08000000, LENGTH = 16k
	approm     (rx) : ORIGIN = 0x08004000, LENGTH = 32k /* Sectors 1-2 */
	ota_flash  (rx) : ORIGIN = 0x0800C000, LENGTH = 16k /* Sector 3 */
	sram	  (rxw) : ORIGIN = 0x20000000, LENGTH = 128k
}

_estack = ORIGIN(sram) + LENGTH(sram);
__app_start = ORIGIN(approm);
__sota_flash = ORIGIN(ota_flash);

SECTIONS {
	.vectors : { KEEP(*(.vectors*)) } > bootloader
	.text	 : {
		*(.text)
		*(.text*)
	} > bootloader
	.rodata  : {
		*(.rodata)
		*(.rodata*)
	} > bootloader

	.data	 : {
		_sdata = .;
		*(.data)
		*(SORT(.data.*))
		_edata = .;
	} > sram AT > bootloader
	_sidata = LOADADDR(.data);

	.bss	 : {
		_sbss = .;
		*(.bss)
		*(SORT(.bss.*))
		*(COMMON)
		_ebss = .;
	} > sram

	.magic_ota_byte 0x20010000 (NOLOAD) : {
		*(.magic_ota_byte*)
	} > sram
	__magic_ota_byte = ADDR(.magic_ota_byte);

	. = ALIGN(8); /* For sbrk */
	_end = .;
}

