
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <tty.h>
#include <kprintf.h>
#include <idt.h>

// panic: Makes a panic message
// Param:	char *string - string to display
// Return:	Nothing

void panic(char *string)
{
	registers_t registers;
	save_registers(&registers);

	debug_mode = 1;
	tty_switch(0);

	kprintf("KERNEL PANIC: %s\n", string);
	dump_registers(&registers);

	while(1)
	{
		asm __volatile__ ("cli\nhlt");
	}
}

// dump_registers: Dumps registers
// Param:	registers_t *registers - pointer to register structure
// Return:	Nothing

void dump_registers(registers_t *registers)
{
	kprintf(" --- BEGINNING REGISTER DUMP AT TIME OF ERROR ---\n");

#if __i386__
	kprintf("  eax: %xd  ebx: %xd  ecx: %xd  edx: %xd\n", registers->eax, registers->ebx, registers->ecx, registers->edx);
	kprintf("  esi: %xd  edi: %xd  esp: %xd  ebp: %xd\n", registers->esi, registers->edi, registers->esp, registers->ebp);
	kprintf("  eflags: %xd  cs: %xw  ss: %xw  ds: %xw  es: %xw\n", registers->eflags, registers->cs, registers->ss, registers->ds, registers->es);
	kprintf("  cr0: %xd  cr2: %xd  cr3: %xd  cr4: %xd\n", registers->cr0, registers->cr2, registers->cr3, registers->cr4);
#endif

#if __x86_64__
	kprintf("  rax: %xq  rbx: %xq  rcx: %xq  rdx: %xq\n", registers->rax, registers->rbx, registers->rcx, registers->rdx);
#endif
	kprintf(" --- END OF REGISTER DUMP ---\n");
}

// install_exceptions(): Installs exception handlers
// Param:	Nothing
// Return:	Nothing

void install_exceptions()
{
	idt_install(0, (uint32_t)&divide_handler);
	idt_install(1, (uint32_t)&debug_handler);
	idt_install(2, (uint32_t)&nmi_handler);
	idt_install(3, (uint32_t)&breakpoint_handler);
	idt_install(4, (uint32_t)&overflow_handler);
	idt_install(5, (uint32_t)&bound_handler);
	idt_install(6, (uint32_t)&opcode_handler);
	idt_install(7, (uint32_t)&device_handler);
	idt_install(8, (uint32_t)&double_handler);
	idt_install(9, (uint32_t)&coprocessor_handler);
	idt_install(10, (uint32_t)&tss_handler);
	idt_install(11, (uint32_t)&segment_handler);
	idt_install(12, (uint32_t)&stack_handler);
	idt_install(13, (uint32_t)&gpf_handler);
	idt_install(14, (uint32_t)&page_handler);
	idt_install(15, (uint32_t)&reserved_handler);
	idt_install(16, (uint32_t)&floating_handler);
	idt_install(17, (uint32_t)&alignment_handler);
	idt_install(18, (uint32_t)&machine_handler);
	idt_install(19, (uint32_t)&simd_handler);
	idt_install(20, (uint32_t)&virtual_handler);
	idt_install(21, (uint32_t)&reserved_handler);
	idt_install(22, (uint32_t)&reserved_handler);
	idt_install(23, (uint32_t)&reserved_handler);
	idt_install(24, (uint32_t)&reserved_handler);
	idt_install(25, (uint32_t)&reserved_handler);
	idt_install(26, (uint32_t)&reserved_handler);
	idt_install(27, (uint32_t)&reserved_handler);
	idt_install(28, (uint32_t)&reserved_handler);
	idt_install(29, (uint32_t)&reserved_handler);
	idt_install(30, (uint32_t)&security_handler);
	idt_install(31, (uint32_t)&reserved_handler);
}

// exception_handler(): Common code for exception handlers
// Param:	char *string - text describing exception
// Param:	uint32_t code - error code, if any
// Return:	Nothing

void exception_handler(char *string, uint32_t code)
{
	registers_t registers;
	save_registers(&registers);

	debug_mode = 1;
	tty_switch(0);

	kprintf("KERNEL PANIC: %s, error code 0x%xd\n", string, code);
	dump_registers(&registers);

	while(1);
}




