OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386")
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
		*(.rodata)
		. = ALIGN(32);
	}
	__bss_start = .;
	.bss : {
		bss = .; _bss = .; __bss = .;
		*(.bss);
	}
	kend = ., _kend = ., __kend = .;
}

