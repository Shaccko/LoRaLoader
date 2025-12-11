ENTRY(_reset);
MEMORY {
	flash(rx) : ORIGIN = 0x08004000, LENGTH = 32k /* Occupying sectors 1-2 */
	ota_flash(rx) : ORIGIN = 0x0800C000, LENGTH = 16k /* Sector 3 */
	sram(rxw) : ORIGIN = 0x20000000, LENGTH = 128k
}
_estack = ORIGIN(sram) + LENGTH(sram);
_sota_flash = ORIGIN(ota_flash);

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

	. = ALIGN(8);
	_end = .;
}
