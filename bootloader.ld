ENTRY(__bootloader_reset);
MEMORY {
	bootloader(rx) : ORIGIN = 0x08000000, LENGTH = 16k 
	flash(rx) :      ORIGIN = 0x08004000, LENGTH = 248k
	sram(rxw) :        ORIGIN = 0x20000000, LENGTH = 128k

	flash_swap(rx) : ORIGIN = 0x08040000, LENGTH = 128k 
	flash_backup(rx) : ORIGIN = 0x08060000, LENGTH = 128k
}

_sflash = ORIGIN(flash);
_eflash = ORIGIN(flash) + LENGTH(flash);
_sflash_swap = ORIGIN(flash_swap);
_eflash_swap = ORIGIN(flash_swap) + LENGTH(flash_swap);
_sflash_backup = ORIGIN(_sflash_backup);
_eflash_backup = ORIGIN(_sflash_backup) + LENGTH(_sflash_backup);

_estack = ORIGIN(sram) + LENGTH(sram);

SECTIONS {
	.vectors : { KEEP(*(.vectors*)) } > bootloader
	.text	 : {
		_stext = .;
		*(.text)
		*(.text*)
		_etext = .;
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

	. = ALIGN(8); /* For sbrk */
	_end = .;
}

