
	CC=clang
	CFLAGS=-Wall -fno-builtin -ffreestanding -fomit-frame-pointer -nostdlib -nodefaultlibs -O2 -msse2
	CFILES=kernel/*.c kernel/*/*.c
	OBJECTS=*.o

lux32:
	rm -f *.o
	$(CC) $(CFLAGS) -target i386-pc-none -Ikernel/include -c runtime/*.c
	ar r runtime.a *.o
	rm -f *.o

	fasm kernel/asm_i386/vbe.asm vbe.sys
	fasm kernel/asm_i386/bootstrap.asm bootstrap.o
	fasm kernel/asm_i386/io.asm io.o
	fasm kernel/asm_i386/state.asm state.o
	fasm kernel/asm_i386/cpu.asm cpu.o
	fasm kernel/asm_i386/sse2.asm sse2.o
	fasm kernel/asm_i386/irq_stub.asm irq_stub.o
	$(CC) $(CFLAGS) -target i386-pc-none -Ikernel/include -c $(CFILES)
	ld -melf_i386 -nostdlib -nodefaultlibs -O2 -T kernel/ld_i386.ld $(OBJECTS) *.a -o iso/boot/kernel.sys

	cd initrd; tar --owner=root --group=root -cf ../iso/boot/initrd.img *; cd ..

	grub-mkrescue -o lux.iso iso

	qemu-system-i386 -enable-kvm -vga std -smp 2 -cdrom lux.iso

lux64:
	rm -f *.o
	fasm kernel/asm_i386/vbe.asm vbe.sys
	fasm kernel/asm_x86_64/bootstrap.asm bootstrap.o
	fasm kernel/asm_x86_64/io.asm io.o
	fasm kernel/asm_x86_64/state.asm state.o
	fasm kernel/asm_x86_64/cpu.asm cpu.o
	fasm kernel/asm_x86_64/sse2.asm sse2.o
	fasm kernel/asm_x86_64/irq_stub.asm irq_stub.o
	$(CC) $(CFLAGS) -target x86_64-pc-none -m64 -mno-red-zone -mcmodel=large -Ikernel/include -c $(CFILES)
	ld -melf_x86_64 -nostdlib -nodefaultlibs -O2 -T kernel/ld_x86_64.ld $(OBJECTS) -o kernel64.sys

	cd initrd; tar --owner=root --group=root -cf ../iso/boot/initrd.img *; cd ..

	fasm kernel/asm_x86_64/setup.asm iso/boot/kernel.sys
	grub-mkrescue -o lux.iso iso
	qemu-system-x86_64 -enable-kvm -vga std -smp 2 -cdrom lux.iso

clean:
	rm -f iso/boot/*.*
	rm -f *.o *.a *.sys



