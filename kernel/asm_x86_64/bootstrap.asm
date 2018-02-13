
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf64
use64

section '.startup'

public start64
start64:
	cli
	cld
	lgdt [gdtr]

	push 0x28
	mov rdx, .next
	push rdx
	retf

.next:
	mov dx, 0x30
	mov ss, dx
	mov ds, dx
	mov es, dx
	mov fs, dx
	mov gs, dx
	mov rsp, stack_top

	; void kmain(uint32_t multiboot_magic, multiboot_info_t *multiboot_info, vbe_mode_t *vbe_mode)
	extrn kmain

	mov rdi, rax
	mov rsi, rbx
	mov rdx, rcx
	call kmain

.halt:
	cli
	hlt
	jmp .halt

section '.rodata' align 16

public trampoline16
trampoline16:
	cli
	hlt

end_trampoline16:

public trampoline16_size
trampoline16_size:		dw end_trampoline16 - trampoline16

align 16
gdt:
	; null descriptor 0x00
	dq 0

	; 32-bit code descriptor 0x08
	dw 0xFFFF				; limit low
	dw 0					; base low
	db 0					; base middle
	db 10011010b				; access
	db 11001111b				; flags and limit high
	db 0					; base high

	; 32-bit data descriptor 0x10
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0

	; 16-bit code descriptor 0x18
	dw 0xFFFF
	dw 0
	db 0
	db 10011010b
	db 10001111b
	db 0

	; 16-bit data descriptor 0x20
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 10001111b
	db 0

	; 64-bit kernel code descriptor 0x28
	dw 0xFFFF
	dw 0
	db 0
	db 10011010b
	db 10101111b
	db 0

	; 64-bit kernel data descriptor 0x30
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 10101111b
	db 0

	; 64-bit user code descriptor 0x38
	dw 0xFFFF
	dw 0
	db 0
	db 11111010b
	db 10101111b
	db 0

	; 64-bit user data descriptor 0x40
	dw 0xFFFF
	dw 0
	db 0
	db 11110010b
	db 10101111b
	db 0

end_of_gdt:

align 16
gdtr:
	dw end_of_gdt - gdt - 1
	dq gdt

public bootfont
bootfont:			file "kernel/asm_i386/cp437.bin"

section '.bss' align 16

align 16
stack_bottom:			rb 16384
stack_top:


