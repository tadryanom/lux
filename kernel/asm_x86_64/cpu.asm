
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

; void flush_gdt(gdtr_t *, uint16_t, uint16_t)
public flush_gdt
flush_gdt:
	cli
	cld
	lgdt [rdi]

	push rsi
	mov rsi, .next
	push rsi
	retf

.next:
	mov ss, dx
	mov ds, dx
	mov es, dx
	mov fs, dx
	mov gs, dx

	ret

; void write_msr(uint32_t, uint64_t)
public write_msr
write_msr:
	mov rcx, rdi
	mov rax, rsi
	mov rdx, rax
	shr rdx, 32
	wrmsr

	ret

; uint64_t read_msr(uint32_t)
public read_msr
read_msr:
	mov rcx, rdi
	rdmsr
	mov rcx, 0
	not ecx
	and rax, rcx
	shl rdx, 32
	or rax, rdx

	ret

; For exceptions
extrn exception_handler

public divide_handler
divide_handler:
	mov rsi, 0
	mov rdi, divide_text
	call exception_handler

	iret

public debug_handler
debug_handler:
	mov rsi, 0
	mov rdi, debug_text
	call exception_handler

	iret

public nmi_handler
nmi_handler:
	mov rsi, 0
	mov rdi, nmi_text
	call exception_handler

	iret

public breakpoint_handler
breakpoint_handler:
	mov rsi, 0
	mov rdi, breakpoint_text
	call exception_handler

	iret

public overflow_handler
overflow_handler:
	mov rsi, 0
	mov rdi, overflow_text
	call exception_handler

	iret

public bound_handler
bound_handler:
	mov rsi, 0
	mov rdi, bound_text
	call exception_handler

	iret

public opcode_handler
opcode_handler:
	mov rsi, 0
	mov rdi, opcode_text
	call exception_handler

	iret

public device_handler
device_handler:
	mov rsi, 0
	mov rdi, device_text
	call exception_handler

	iret

public double_handler
double_handler:
	pop rsi
	mov rdi, double_text
	call exception_handler

	iret

public coprocessor_handler
coprocessor_handler:
	mov rsi, 0
	mov rdi, coprocessor_text
	call exception_handler

	iret

public tss_handler
tss_handler:
	pop rsi
	mov rdi, tss_text
	call exception_handler

	iret

public segment_handler
segment_handler:
	pop rsi
	mov rdi, segment_text
	call exception_handler

	iret

public stack_handler
stack_handler:
	pop rsi
	mov rdi, stack_text
	call exception_handler

	iret

public gpf_handler
gpf_handler:
	pop rsi
	mov rdi, gpf_text
	call exception_handler

	iret

public page_handler
page_handler:
	pop rsi
	mov rdi, page_text
	call exception_handler

	iret

public reserved_handler
reserved_handler:
	mov rsi, 0
	mov rdi, reserved_text
	call exception_handler

	iret

public floating_handler
floating_handler:
	mov rsi, 0
	mov rdi, floating_text
	call exception_handler

	iret

public alignment_handler
alignment_handler:
	pop rsi
	mov rdi, alignment_text
	call exception_handler

	iret

public machine_handler
machine_handler:
	mov rsi, 0
	mov rdi, machine_text
	call exception_handler

	iret

public simd_handler
simd_handler:
	mov rsi, 0
	mov rdi, simd_text
	call exception_handler

	iret

public virtual_handler
virtual_handler:
	mov rsi, 0
	mov rdi, virtual_text
	call exception_handler

	iret

public security_handler
security_handler:
	pop rsi
	mov rdi, security_text
	call exception_handler

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



