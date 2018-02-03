
	gccopts = -m32 -nostdlib -nostartfiles -nodefaultlibs -fomit-frame-pointer -O2

all:
	if [ ! -d "out" ]; then mkdir out; fi
	if [ ! -d "out/kernel" ]; then mkdir out/kernel; fi

	fasm kernel/asm/vbe.asm out/kernel/vbe.sys
	fasm kernel/asm/bootstrap.asm out/kernel/bootstrap.o
	fasm kernel/asm/io.asm out/kernel/io.o
	fasm kernel/asm/state.asm out/kernel/state.o
	fasm kernel/asm/cpu.asm out/kernel/cpu.o
	fasm kernel/asm/sse2.asm out/kernel/sse2.o

	gcc $(gccopts) -Ikernel/include -c kernel/kmain.c -o out/kernel/kmain.o
	gcc $(gccopts) -Ikernel/include -c kernel/misc/string.c -o out/kernel/string.o
	gcc $(gccopts) -Ikernel/include -c kernel/misc/kprintf.c -o out/kernel/kprintf.o
	gcc $(gccopts) -Ikernel/include -c kernel/firmware/pit.c -o out/kernel/pit.o
	gcc $(gccopts) -Ikernel/include -c kernel/mm/mm.c -o out/kernel/mm.o
	gcc $(gccopts) -Ikernel/include -c kernel/mm/pmm.c -o out/kernel/pmm.o
	gcc $(gccopts) -Ikernel/include -c kernel/mm/vmm.c -o out/kernel/vmm.o
	gcc $(gccopts) -Ikernel/include -c kernel/mm/heap.c -o out/kernel/heap.o
	gcc $(gccopts) -Ikernel/include -c kernel/gdi/screen.c -o out/kernel/screen.o

	gcc $(gccopts) -T kernel/link.ld out/kernel/*.o -o iso/boot/kernel.sys

	grub-mkrescue -o lux.iso iso

	qemu-system-i386 -cdrom lux.iso -serial stdio -vga std

clean:
	if [ -d "out/kernel" ]; then rm out/kernel/*; rmdir out/kernel; fi
	if [ -d "out" ]; then rmdir out; fi
	rm iso/boot/kernel.sys
