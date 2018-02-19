
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf64
use64

section '.text'

macro irq_enter
{
	cli
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
}

macro irq_exit
{
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax
}

public pic0_spurious_stub
pic0_spurious_stub:
	irq_enter

	extrn pic0_spurious
	call pic0_spurious

	irq_exit
	iretq

public pic1_spurious_stub
pic1_spurious_stub:
	irq_enter

	extrn pic1_spurious
	call pic1_spurious

	irq_exit
	iretq

public lapic_spurious_stub
lapic_spurious_stub:
	irq_enter

	extrn lapic_spurious
	call lapic_spurious

	irq_exit
	iretq

public timer_irq_stub
timer_irq_stub:
	irq_enter

	extrn timer_irq
	call timer_irq

	irq_exit
	iretq




