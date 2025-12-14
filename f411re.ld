ENTRY(_reset);
MEMORY {
	flash(rx) : ORIGIN = 0x08004000, LENGTH = 32k /* Occupying sectors 1-2 */
	ota_flash(rx) : ORIGIN = 0x0800C000, LENGTH = 16k /* Sector 3 */
	sram(rxw) : ORIGIN = 0x20000000, LENGTH = 128k
}
_estack = ORIGIN(sram) + LENGTH(sram);

SECTIONS {
	.vectors : { KEEP(*(.vectors*)) } > flash
	.rodata : {
		*(.rodata)
		*(.rodata*)
	} > flash
	.text : {
		*(.text)
		*(.text*)
	} > flash


	.data : {
		_sdata = .;
		*(.data)
		*(.data*)
		_edata = .;
	} > sram AT > flash
	_sidata = LOADADDR(.data);

	.bss : {
		_sbss = .;
		*(.bss)
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > sram

	.magic_ota_byte 0x20010000(NOLOAD) : {
		*(.magic_ota_byte*)
	} > sram
	__magic_ota_byte = ADDR(.magic_ota_byte);

	. = ALIGN(8);
	_end = .;
}
