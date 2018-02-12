OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386:x86-64")
ENTRY(start)
SECTIONS
{
	. = 0x10000200000;
	.text : {
		*(.text)
		. = ALIGN(32);
	}
	.data : {
		*(.data)
		. = ALIGN(32);
	}
	.rodata : {
		*(.rodata)
		. = ALIGN(32);
	}
	.bss : {
		*(COMMON)
		*(.bss);
	}
	kend = ., _kend = ., __kend = .;
}


