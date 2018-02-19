
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf64
use64

section '.startup'

; Paging Structures
PML4				= 0x30000
PDPT				= 0x31000
PDIR				= 0x32000

public start64
start64:
	cli
	cld
	mov rdx, gdtr
	lgdt [rdx]

	push 0x18
	mov rdx, .next
	push rdx
	retf

.next:
	mov dx, 0x20
	mov ss, dx
	mov ds, dx
	mov es, dx
	mov fs, dx
	mov gs, dx
	mov rsp, stack_top

	mov rdx, idtr
	lidt [rdx]

	push 2
	popfq

	finit
	fwait

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
use16
	xor ax, ax
	mov ds, ax
	mov es, ax

	lgdt [0x1800]	; temporary GDT just for switching
			; we can't use the real GDT because it's around 1024 GB
			; and we can't access that in real mode and pmode

	; enable PAE, SSE and PSE
	mov eax, 0x630
	mov cr4, eax

	mov eax, cr0
	and eax, 0xFFFFFFFB
	or eax, 2
	mov cr0, eax

	; go to long mode
	mov eax, PML4
	mov cr3, eax

	mov ecx, 0xC0000080
	rdmsr
	or eax, 0x100
	wrmsr

	mov eax, cr0
	or eax, 0x80000001
	and eax, not 0x60000000
	mov cr0, eax
	jmp 0x08:0x1100

times 256 - ($-trampoline16) db 0

use64
trampoline64:
	mov ax, 0x10
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov rsp, ap_stack_top

	mov rdx, gdtr
	lgdt [rdx]

	push 0x18
	push 0x1200
	retf

times 512 - ($-trampoline16) db 0

	mov ax, 0x20
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov rsp, ap_stack_top

	mov rdx, idtr
	lidt [rdx]

	push 2
	popfq

	finit
	fwait

	extrn kmalloc
	mov rdi, 16384		; temporary stack
	mov rax, kmalloc
	call rax

	mov rsp, rax
	add rsp, 16384

	extrn smp_kmain
	mov rax, smp_kmain
	jmp rax

times 1024 - ($-trampoline16) db 0
tmp_gdt:
	; null descriptor 0x00
	dq 0

	; 64-bit kernel code descriptor 0x08
	dw 0xFFFF
	dw 0
	db 0
	db 10011010b
	db 10101111b
	db 0

	; 64-bit kernel data descriptor 0x10
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 10101111b
	db 0

end_tmp_gdt:

times 2048 - ($-trampoline16) db 0
tmp_gdtr:
	dw end_tmp_gdt - tmp_gdt - 1
	dq 0x1400

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

	; 64-bit kernel code descriptor 0x18
	dw 0xFFFF
	dw 0
	db 0
	db 10011010b
	db 10101111b
	db 0

	; 64-bit kernel data descriptor 0x20
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 10101111b
	db 0

	; 64-bit user code descriptor 0x28
	dw 0xFFFF
	dw 0
	db 0
	db 11111010b
	db 10101111b
	db 0

	; 64-bit user data descriptor 0x30
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

public idt
align 16
idt:
	times 256 dw 0, 0x18, 0x8E00, 0, 0, 0, 0, 0

end_of_idt:

public idtr
align 16
idtr:
	dw end_of_idt - idt - 1
	dq idt

public bootfont
bootfont:			file "kernel/asm_i386/cp437.bin"

section '.bss' align 16

align 16
stack_bottom:			rb 16384
stack_top:

align 16
ap_stack_bottom:		rb 16384
ap_stack_top:



