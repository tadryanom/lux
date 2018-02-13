OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386:x86-64")
ENTRY(start64)
SECTIONS
{
	. = 0x10000200000;
	.startup : {
		*(.startup)
		. = ALIGN(8);
	}
	.text : {
		*(.text)
		. = ALIGN(8);
	}
	.data : {
		*(.data)
		. = ALIGN(8);
	}
	.rodata : {
		*(.rodata)
		. = ALIGN(8);
	}
	.bss : {
		*(COMMON)
		*(.bss);
	}
	kend = ., _kend = ., __kend = .;
}


