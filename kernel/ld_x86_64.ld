OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386:x86-64")
ENTRY(start64)
SECTIONS
{
	. = 0x10000200000;
	.startup : {
		*(.startup)
		. = ALIGN(16);
	}
	.text : {
		*(.text)
		. = ALIGN(16);
	}
	.data : {
		*(.data)
		. = ALIGN(16);
	}
	.rodata : {
		*(.rodata)
		. = ALIGN(16);
	}
	.bss : {
		*(COMMON)
		*(.bss);
	}
}


