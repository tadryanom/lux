
	CC=clang
	CFLAGS=-target i386-elf -fno-builtin -ffreestanding -fomit-frame-pointer -nostdlib -nodefaultlibs -O2

all:
	if [ ! -d "out" ]; then mkdir out; fi
	if [ ! -d "out/kernel" ]; then mkdir out/kernel; fi

	fasm kernel/asm/vbe.asm out/kernel/vbe.sys
	fasm kernel/asm/bootstrap.asm out/kernel/bootstrap.o
	fasm kernel/asm/io.asm out/kernel/io.o
	fasm kernel/asm/state.asm out/kernel/state.o
	fasm kernel/asm/cpu.asm out/kernel/cpu.o
	fasm kernel/asm/sse2.asm out/kernel/sse2.o

	$(CC) $(CFLAGS) -Ikernel/include -c kernel/kmain.c -o out/kernel/kmain.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/misc/string.c -o out/kernel/string.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/misc/kprintf.c -o out/kernel/kprintf.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/misc/idt.c -o out/kernel/idt.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/misc/panic.c -o out/kernel/panic.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/firmware/pit.c -o out/kernel/pit.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/mm/mm.c -o out/kernel/mm.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/mm/pmm.c -o out/kernel/pmm.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/mm/vmm.c -o out/kernel/vmm.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/mm/heap.c -o out/kernel/heap.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/io/tty.c -o out/kernel/tty.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/acpi/tables.c -o out/kernel/tables.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/acpi/apic.c -o out/kernel/apic.o
	$(CC) $(CFLAGS) -Ikernel/include -c kernel/acpi/ioapic.c -o out/kernel/ioapic.o

	ld -melf_i386 -nostdlib -nodefaultlibs -O2 -T kernel/link.ld out/kernel/*.o -o iso/boot/kernel.sys

	grub-mkrescue -o lux.iso iso

	qemu-system-i386 -cdrom lux.iso -serial stdio -vga std

clean:
	if [ -d "out/kernel" ]; then rm out/kernel/*; rmdir out/kernel; fi
	if [ -d "out" ]; then rmdir out; fi
	rm iso/boot/kernel.sys



