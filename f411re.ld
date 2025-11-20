ENTRY(_reset);
MEMORY {
	flash(rx) : ORIGIN = 0x08004000, LENGTH = 512k
	sram(rxw) : ORIGIN = 0x20000000, LENGTH = 128k
}

_estack = ORIGIN(sram) + LENGTH(sram);

SECTIONS {
	.vectors : { (KEEP(.vectors*)) } > flash
	.text : {
		*(.text)
		*(.text*)
	} > flash
	.rodata : { 
		*(.rodata)
		*(.rodata*)
	} > flash

	.data : {
		_sdata = .;
		*(.data)
		*(SORT (.data*))
		_edata = .;
	} > sram AT > flash
	_sidata = LOADADDR(.data);

	.bss : {
		_sbss = .;
		*(.bss)
		*(sort (.bss*))
		(COMMON)
		_bss = .;
	} > sram 
}

