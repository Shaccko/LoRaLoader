ENTRY(__bootloader_reset);
MEMORY {
	bootloader(rx): ORIGIN = 0x08000000, LENGTH = 16k /* Bootloader, sector 0 */
	flash_md(rx) :  ORIGIN = 0x08004000, LENGTH = 16k
	flash_a(rx) : 	ORIGIN = 0x08008000, LENGTH = 248k - 16k /* Occupying sectors 2-5 */
	flash_b(rx) :   ORIGIN = 0x08040000, LENGTH = 248k /* Sector 6-7 */
	sram(rxw) : 	ORIGIN = 0x20000000, LENGTH = 128k
}

_sflash_a =  ORIGIN(flash_a);
_sflash_b =  ORIGIN(flash_b);
_estack   =  ORIGIN(sram) + LENGTH(sram);

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

	.flash_metadata : {
		_flash_ptr = .;
		KEEP(*(.flash_metadata))
	} > flash_md

	.bss	 : {
		_sbss = .;
		*(.bss)
		*(SORT(.bss.*))
		*(COMMON)
		_ebss = .;
	} > sram

	. = ALIGN(8); /* For sbrk */
	_end = .;
}

