INCLUDE memory_map.ld

SECTIONS {
	.text : {
		KEEP(*(.vectors .vectors.*))
		*(.text*)
		*(.rodata*)
		_etext = .;
	} > bootrom
}
