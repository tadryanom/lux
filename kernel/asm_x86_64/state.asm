
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf64
use64

section '.text'

; void save_registers(registers_t *registers)
; Saves machine state to a structure

public save_registers
save_registers:
	mov [tmp_struct.rax], rax
	mov [tmp_struct.rbx], rbx
	mov [tmp_struct.rcx], rcx
	mov [tmp_struct.rdx], rdx
	mov [tmp_struct.rsi], rsi
	mov [tmp_struct.rdi], rdi
	mov [tmp_struct.rsp], rsp
	mov [tmp_struct.rbp], rbp

	pushfq
	pop rax
	mov [tmp_struct.rflags], rax

	mov ax, cs
	mov word[tmp_struct.cs], ax

	mov ax, ss
	mov word[tmp_struct.ss], ax

	mov ax, ds
	mov word[tmp_struct.ds], ax

	mov ax, es
	mov word[tmp_struct.es], ax

	mov rax, cr0
	mov [tmp_struct.cr0], rax
	mov rax, cr2
	mov [tmp_struct.cr2], rax
	mov rax, cr3
	mov [tmp_struct.cr3], rax
	mov rax, cr4
	mov [tmp_struct.cr4], rax

	; copy to the destination
	mov rdi, rsi			; register_t *
	mov rsi, tmp_struct
	mov rcx, end_tmp_struct-tmp_struct
	rep movsb

	ret

section '.data'

tmp_struct:
	.rax			dq 0
	.rbx			dq 0
	.rcx			dq 0
	.rdx			dq 0
	.rsi			dq 0
	.rdi			dq 0
	.rsp			dq 0
	.rbp			dq 0
	.r8			dq 0
	.r9			dq 0
	.r10			dq 0
	.r11			dq 0
	.r12			dq 0
	.r13			dq 0
	.r14			dq 0
	.r15			dq 0
	.rflags			dq 0
	.cs			dq 0
	.ss			dq 0
	.ds			dq 0
	.es			dq 0
	.cr0			dq 0
	.cr2			dq 0
	.cr3			dq 0
	.cr4			dq 0
end_tmp_struct:




