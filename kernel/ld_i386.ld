
OUTPUT_ARCH("i386")
ENTRY(start)
SECTIONS
{
	. = 0x100000;
	.multiboot : {
		*(.multiboot)
		. = ALIGN(32);
	}

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

