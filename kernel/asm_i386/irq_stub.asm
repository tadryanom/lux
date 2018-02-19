
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf
use32

section '.text'

macro irq_enter
{
	cli
	pusha
}

macro irq_exit
{
	popa
}

public pic0_spurious_stub
pic0_spurious_stub:
	irq_enter

	extrn pic0_spurious
	call pic0_spurious

	irq_exit
	iret

public pic1_spurious_stub
pic1_spurious_stub:
	irq_enter

	extrn pic1_spurious
	call pic1_spurious

	irq_exit
	iret

public lapic_spurious_stub
lapic_spurious_stub:
	irq_enter

	extrn lapic_spurious
	call lapic_spurious

	irq_exit
	iret

public timer_irq_stub
timer_irq_stub:
	irq_enter

	extrn timer_irq
	call timer_irq

	irq_exit
	iret





