ENTRY(_bootloader);
MEMORY {
	bootloader (rx) : ORIGIN = 0x08000000, LENGTH = 16k
	app	   (rx) : ORIGIN = 0x08004000, LENGTH = 512k - 16k
	sram	  (rxw) : ORIGIN = 0x20000000, LENGTH = 128k
}

_startapp = ORIGIN(app);
_startapp_size = LENGTH(app);

_estack = ORIGIN(sram) + LENGTH(sram);

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
		_sdata	= .;
		*(.data)
		*(SORT(.data*))
		_edata	= .;
	} > sram > bootloader
	_sidata = LOADADDR(.data)

	.bss	 : {
		_sbss 	= .;
		*(.bss)
		*(SORT(.bss*))
		(COMMON)
		_edata 	= .;
	} > sram
}


