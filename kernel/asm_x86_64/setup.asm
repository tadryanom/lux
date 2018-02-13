
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

org LOAD_ADDRESS
use32

LOAD_ADDRESS			= 0x100000	; 1 MB
MULTIBOOT_MAGIC			= 0x1BADB002
MULTIBOOT_FLAGS			= 0x00010003	; page-align modules, E820 map, non-ELF kernel
MULTIBOOT_CHECKSUM		= -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

multiboot_magic			dd MULTIBOOT_MAGIC
multiboot_flags			dd MULTIBOOT_FLAGS
multiboot_checksum		dd MULTIBOOT_CHECKSUM

multiboot_header		dd multiboot_magic
multiboot_load_address		dd LOAD_ADDRESS
multiboot_end_address		dd 0
multiboot_bss			dd 0
multiboot_entry			dd start

; Paging Structures
PML4				= 0x30000
PDPT				= 0x31000
PDIR				= 0x32000

; start:
; 32-bit entry point from multiboot

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

	mov [saved_magic], eax
	mov [saved_structure], ebx

	; check for long mode
	mov eax, 0x80000000
	cpuid

	cmp eax, 0x80000001
	jl no64

	mov eax, 0x80000001
	cpuid

	test edx, 0x20000000
	jz no64

	; copy 16-bit VBE driver to low memory
	mov esi, vbe_driver
	mov edi, 0x1000
	mov ecx, end_vbe_driver - vbe_driver
	rep movsb

	; return to 16-bit mode to set VBE mode
	jmp 0x18:.next16

use16
.next16:
	mov ax, 0x20
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
	cli
	cld

	mov ax, 0x10
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov esp, stack_top

	mov [vbe_mode], ebx

	; clear paging structures
	mov edi, PML4
	mov eax, 0
	mov ecx, 1024
	rep stosd

	mov edi, PDPT
	mov ecx, 1024
	rep stosd

	; make the PML4
	mov edi, PML4
	mov eax, PDPT
	or eax, 3
	stosd

	mov edi, PML4
	add edi, 16
	stosd

	; make the PDPT
	mov edi, PDPT
	mov ebx, PDIR
	mov ecx, 64		; map 64 GB of physical mem at 1024 GB virtual

.pdpt_loop:
	mov eax, ebx
	or eax, 3
	stosd
	mov eax, 0
	stosd

	add ebx, 4096
	loop .pdpt_loop

	; make the page directory
	mov edi, PDIR
	mov ebx, 0
	mov edx, 0
	mov ecx, 65536/2	; 64 GB

.pdir_loop:
	mov eax, ebx
	or eax, 0x83
	stosd
	mov eax, edx
	stosd

	add ebx, 0x200000	; 2 MB pages
	cmp ebx, 0
	je .add_high

	loop .pdir_loop
	jmp .paging_finish

.add_high:
	inc edx
	loop .pdir_loop

.paging_finish:
	; copy the 64-bit kernel to 2 MB physical
	mov esi, kernel
	mov edi, 0x200000
	mov ecx, end_kernel-kernel
	rep movsb

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
	or eax, 0x80000000
	and eax, not 0x60000000
	mov cr0, eax
	jmp 0x28:start64

no64:
	mov esi, .msg
	jmp do_error

.msg			db "Boot error: CPU is not 64-bit capable.",0

; do_error:
; Displays an error message
; In\	ESI = String to display
; Out\	Nothing

do_error:
	mov edi, 0xB8000
	mov ax, 0x0000
	mov ecx, 80*25
	rep stosw

	mov edi, 0xB8000

.loop:
	lodsb
	cmp al, 0
	je .done

	stosb
	mov al, 0x07
	stosb
	jmp .loop

.done:
	cli
	hlt
	jmp .done

use64

start64:
	mov ax, 0x30
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov rax, 0
	not eax
	and rsp, rax

	xor rax, rax
	xor rbx, rbx
	xor rcx, rcx

	mov eax, [saved_magic]
	mov ebx, [saved_structure]
	mov ecx, [vbe_mode]
	
	mov rbp, 0x10000200000
	jmp rbp				; kernel entry point

; gdt:
; Global Descriptor Table
align 32
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

align 32
gdtr:
	dw end_of_gdt - gdt - 1
	dq gdt

align 32
stack_bottom:			rb 8192
stack_top:

saved_magic			dd 0
saved_structure			dd 0
vbe_mode			dd 0

; VBE Driver
vbe_driver:			file "vbe.sys"
end_vbe_driver:

kernel:				file "kernel64.sys"
end_kernel:



