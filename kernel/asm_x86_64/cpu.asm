
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf64
use64

section '.text'

; void write_cr0(uint32_t value)
public write_cr0
write_cr0:
	mov rax, rdi
	mov cr0, rax
	ret

; void write_cr3(uint32_t value)
public write_cr3
write_cr3:
	mov rax, rdi
	mov cr3, rax
	ret

; void write_cr4(uint32_t value)
public write_cr4
write_cr4:
	mov rax, rdi
	mov cr4, rax
	ret

; uint32_t read_cr0()
public read_cr0
read_cr0:
	mov rax, cr0
	ret

; uint32_t read_cr2()
public read_cr2
read_cr2:
	mov rax, cr2
	ret

; uint32_t read_cr3()
public read_cr3
read_cr3:
	mov rax, cr3
	ret

; uint32_t read_cr4()
public read_cr4
read_cr4:
	mov rax, cr4
	ret

; void flush_tlb(size_t base, size_t count)
public flush_tlb
flush_tlb:
	mov rax, rdi
	mov rcx, rsi

	cmp rcx, 0
	je .done

.loop:
	invlpg [rax]
	add rax, 4096
	loop .loop

.done:
	ret

; void acquire_lock(lock_t *)
public acquire_lock
acquire_lock:
	mov rax, rdi		; lock_t *

.loop:
	bt qword[rax], 0
	jc .loop

	lock bts qword[rax], 0
	jc .loop

	ret

; void release_lock(lock_t *)
public release_lock
release_lock:
	mov rax, rdi
	btr qword[rax], 0
	ret

; For exceptions
extrn exception_handler

public divide_handler
divide_handler:
	push 0
	mov r15, divide_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public debug_handler
debug_handler:
	push 0
	mov r15, debug_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public nmi_handler
nmi_handler:
	push 0
	mov r15, nmi_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public breakpoint_handler
breakpoint_handler:
	push 0
	mov r15, breakpoint_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public overflow_handler
overflow_handler:
	push 0
	mov r15, overflow_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public bound_handler
bound_handler:
	push 0
	mov r15, bound_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public opcode_handler
opcode_handler:
	push 0
	mov r15, opcode_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public device_handler
device_handler:
	push 0
	mov r15, device_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public double_handler
double_handler:
	mov r15, double_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public coprocessor_handler
coprocessor_handler:
	push 0
	mov r15, coprocessor_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public tss_handler
tss_handler:
	mov r15, tss_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public segment_handler
segment_handler:
	mov r15, segment_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public stack_handler
stack_handler:
	mov r15, stack_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public gpf_handler
gpf_handler:
	mov r15, gpf_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public page_handler
page_handler:
	mov r15, page_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public reserved_handler
reserved_handler:
	push 0
	mov r15, reserved_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public floating_handler
floating_handler:
	push 0
	mov r15, floating_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public alignment_handler
alignment_handler:
	mov r15, alignment_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public machine_handler
machine_handler:
	push 0
	mov r15, machine_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public simd_handler
simd_handler:
	push 0
	mov r15, simd_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public virtual_handler
virtual_handler:
	push 0
	mov r15, virtual_text
	push r15
	call exception_handler

	add rsp, 16
	iret

public security_handler
security_handler:
	mov r15, security_text
	push r15
	call exception_handler

	add rsp, 16
	iret

;section '.rodata'

divide_text			db "Division error",0
debug_text			db "Debug interrupt",0
nmi_text			db "Non-maskable interrupt",0
breakpoint_text			db "Breakpoint",0
overflow_text			db "Overflow exception",0
bound_text			db "BOUND range overflow",0
opcode_text			db "Undefined opcode",0
device_text			db "Device not present",0
double_text			db "Double fault",0
coprocessor_text		db "Coprocessor segment overrun",0
tss_text			db "Corrupt TSS",0
segment_text			db "Memory segment not present",0
stack_text			db "Stack segment error",0
gpf_text			db "General protection fault",0
page_text			db "Page fault",0
reserved_text			db "Reserved exception",0
floating_text			db "x87 floating point error",0
alignment_text			db "Alignment check",0
machine_text			db "Machine check",0
simd_text			db "SIMD floating point error",0
virtual_text			db "Virtualization exception",0
security_text			db "Security exception",0



