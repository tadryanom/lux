
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf
use32

section '.multiboot'

MULTIBOOT_MAGIC			= 0x1BADB002
MULTIBOOT_FLAGS			= 0x00000003	; page-align modules, E820 map
MULTIBOOT_CHECKSUM		= -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

multiboot_magic			dd MULTIBOOT_MAGIC
multiboot_flags			dd MULTIBOOT_FLAGS
multiboot_checksum		dd MULTIBOOT_CHECKSUM

saved_magic			dd 0
saved_structure			dd 0

section '.text'

; start:
; Start of kernel code
public start
start:
	cli
	cld

	lgdt [gdtr]
	jmp 0x08:.next

.next:
	mov dx, 0x10
	mov ss, dx
	mov ds, dx
	mov es, dx
	mov fs, dx
	mov gs, dx
	mov esp, stack_top

	; save multiboot information
	mov [saved_magic], eax
	mov [saved_structure], ebx

	; copy 16-bit VBE driver to low memory
	mov esi, vbe_driver
	mov edi, 0x1000
	mov ecx, end_vbe_driver - vbe_driver
	rep movsb

	; return to 16-bit mode to set VBE mode
	jmp 0x28:.next16

use16
.next16:
	mov ax, 0x30
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, cr0
	and eax, not 1
	mov cr0, eax

	; pass the return address in EBX
	; the 16-bit program will return back to this point, in 32-bit mode
	mov ebx, .vbe_finish
	jmp 0x0000:0x1000

use32
.vbe_finish:
	mov ax, 0x10
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov esp, stack_top

	; load IDT
	lidt [idtr]

	; reset EFLAGS
	push 0x00000002
	popfd

	; enable SSE
	mov eax, 0x600
	mov cr4, eax

	mov eax, cr0
	and eax, 0xFFFFFFFB
	or eax, 2
	mov cr0, eax

	; initialize the FPU
	finit
	fwait

	push ebx		; VBE mode info block

	mov ebx, [saved_structure]
	mov eax, [saved_magic]
	push ebx		; multiboot information
	push eax		; multiboot magic

	extrn kmain
	call kmain		; void kmain(mb_magic, mb_info, vbe_info);

	; we should never, EVER be here!
.halt:
	cli
	hlt
	jmp .halt

section '.rodata' align 16
	
; gdt:
; Global Descriptor Table
align 16
public gdt
gdt:
	; 0x00 -- null descriptor
	dq 0

	; 0x08 -- kernel code descriptor
	dw 0xFFFF				; limit low
	dw 0					; base low
	db 0					; base middle
	db 10011010b				; access
	db 11001111b				; flags and limit high
	db 0					; base high

	; 0x10 -- kernel data descriptor
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0

	; 0x18 -- user code descriptor
	dw 0xFFFF				; limit low
	dw 0					; base low
	db 0					; base middle
	db 11111010b				; access
	db 11001111b				; flags and limit high
	db 0					; base high

	; 0x20 -- user data descriptor
	dw 0xFFFF
	dw 0
	db 0
	db 11110010b
	db 11001111b
	db 0

	; 0x28 -- 16-bit code descriptor
	dw 0xFFFF
	dw 0
	db 0
	db 10011010b
	db 10001111b
	db 0

	; 0x30 -- 16-bit data descriptor
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 10001111b
	db 0

	; 0x38 -- TSS descriptor
	dw 104
	dw 0
	db 0
	db 11101001b
	db 0
	db 0

end_of_gdt:

; gdtr:
; GDT Pointer
align 16
gdtr:
	.size			dw end_of_gdt - gdt - 1
	.base			dd gdt

; idt:
; Interrupt Descriptor Table
align 16
public idt
idt:
	times 256 dw 0, 8, 0x8E00, 0
end_of_idt:

; idtr:
; IDT Pointer
align 16
idtr:
	.size			dw end_of_idt - idt - 1
	.base			dd idt

; VBE Driver
vbe_driver:			file "out/kernel/vbe.sys"
end_vbe_driver:

; Boot-time Font
public bootfont
bootfont:			file "kernel/asm/cp437.bin"

section '.bss' align 16

align 16
stack_bottom:			rb 8192
stack_top:



