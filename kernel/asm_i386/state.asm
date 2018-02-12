
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf
use32

section '.text'

; void save_registers(registers_t *registers)
; Saves machine state to a structure

public save_registers
save_registers:
	pusha

	mov [tmp_struct.eax], eax
	mov [tmp_struct.ebx], ebx
	mov [tmp_struct.ecx], ecx
	mov [tmp_struct.edx], edx
	mov [tmp_struct.esi], esi
	mov [tmp_struct.edi], edi
	mov [tmp_struct.esp], esp
	mov [tmp_struct.ebp], ebp

	pushfd
	pop eax
	mov [tmp_struct.eflags], eax

	mov ax, cs
	mov word[tmp_struct.cs], ax

	mov ax, ss
	mov word[tmp_struct.ss], ax

	mov ax, ds
	mov word[tmp_struct.ds], ax

	mov ax, es
	mov word[tmp_struct.es], ax

	mov eax, cr0
	mov [tmp_struct.cr0], eax
	mov eax, cr2
	mov [tmp_struct.cr2], eax
	mov eax, cr3
	mov [tmp_struct.cr3], eax
	mov eax, cr4
	mov [tmp_struct.cr4], eax

	; copy to the destination
	mov edi, [esp+32+4]		; register_t *
	mov esi, tmp_struct
	mov ecx, end_tmp_struct-tmp_struct
	rep movsb

	popa
	ret

section '.data'

tmp_struct:
	.eax			dd 0
	.ebx			dd 0
	.ecx			dd 0
	.edx			dd 0
	.esi			dd 0
	.edi			dd 0
	.esp			dd 0
	.ebp			dd 0
	.eflags			dd 0
	.cs			dd 0
	.ss			dd 0
	.ds			dd 0
	.es			dd 0
	.cr0			dd 0
	.cr2			dd 0
	.cr3			dd 0
	.cr4			dd 0
end_tmp_struct:




