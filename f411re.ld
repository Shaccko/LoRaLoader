ENTRY(_reset);
MEMORY {
	flash_a(rx) : 	ORIGIN = 0x08004000, LENGTH = 248k /* Occupying sectors 1-5 */
	flash_b(rx) : ORIGIN = 0x08040000, LENGTH = 248k /* Sector 6-7 */
	sram(rxw) : 	ORIGIN = 0x20000000, LENGTH = 128k
}
_sflash_a = ORIGIN(flash_a);
_sflash_b = ORIGIN(flash_b);
_estack = ORIGIN(sram) + LENGTH(sram);

SECTIONS {
	.vectors : { KEEP(*(.vectors*)) } > flash_a
	.rodata : {
		*(.rodata)
		*(.rodata*)
	} > flash_a
	.text : {
		_stext = .;
		*(.text)
		*(.text*)
		_etext = .;
	} > flash_a


	.data : {
		_sdata = .;
		*(.data)
		*(.data*)
		_edata = .;
	} > sram AT > flash_a
	_sidata = LOADADDR(.data);

	.bss : {
		_sbss = .;
		*(.bss)
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > sram

	/* Custom sections */
	.image_metadata 0x20001000(NOLOAD) : {
		__image_curr = .;
		*(.image_curr*)
		. += 4;

		__image_wr_bit = .;
		*(.image_wr_bit*)
		. += 4;
	} > sram

	. = ALIGN(8);
	_end = .;
}
