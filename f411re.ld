ENTRY(_reset);
MEMORY {
	flash_md(rx) :  ORIGIN = 0x08004000, LENGTH = 16k /* Flash metadata */
	flash_a(rx) : 	ORIGIN = 0x08008000, LENGTH = 248k - 16k /* Occupying sectors 1-5 */
	flash_b(rx) :   ORIGIN = 0x08040000, LENGTH = 248k /* Sector 6-7 */
	sram(rxw) : 	ORIGIN = 0x20000000, LENGTH = 128k
}

_sflash_a = ORIGIN(flash_a);
_sflash_b = ORIGIN(flash_b);
_estack = ORIGIN(sram) + LENGTH(sram);
_flash_ptr = ORIGIN(flash_md);

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

	. = ALIGN(8);
	_end = .;
}
