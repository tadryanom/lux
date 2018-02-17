
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <tty.h>
#include <kprintf.h>
#include <idt.h>
#include <cpu.h>

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
	kprintf("  efl: %xd  cs: %xw  ss: %xw  ds: %xw  es: %xw\n", registers->eflags, registers->cs, registers->ss, registers->ds, registers->es);
	kprintf("  cr0: %xd  cr2: %xd  cr3: %xd  cr4: %xd\n", registers->cr0, registers->cr2, registers->cr3, registers->cr4);
#endif

#if __x86_64__
	kprintf("  rax: %xq  rbx: %xq  rcx: %xq\n", registers->rax, registers->rbx, registers->rcx);
	kprintf("  rdx: %xq  rsi: %xq  rdi: %xq\n", registers->rdx, registers->rsi, registers->rdi);
	kprintf("  rsp: %xq  rbp: %xq  rfl: %xq\n", registers->rsp, registers->rbp, registers->rflags);
	kprintf("   r8: %xq  r9:  %xq  r10: %xq\n", registers->r8, registers->r9, registers->r10);
	kprintf("  r11: %xq  r12: %xq  r13: %xq\n", registers->r11, registers->r12, registers->r13);
	kprintf("  r14: %xq  r15: %xq\n", registers->r14, registers->r15);
	kprintf("  cr2: %xq  fs:  %xq  gs:  %xq\n", registers->cr2, read_msr(MSR_FS_BASE), read_msr(MSR_GS_BASE));
	kprintf("  cs: %xw  ss: %xw  ds: %xw  es: %xw\n", registers->cs, registers->ss, registers->ds, registers->es);
	kprintf("  cr0: %xd  cr3: %xq   cr4: %xd\n", registers->cr0, registers->cr3, registers->cr4);
#endif
	kprintf(" --- END OF REGISTER DUMP ---\n");
}

// install_exceptions(): Installs exception handlers
// Param:	Nothing
// Return:	Nothing

void install_exceptions()
{
	idt_install(0, (size_t)&divide_handler);
	idt_install(1, (size_t)&debug_handler);
	idt_install(2, (size_t)&nmi_handler);
	idt_install(3, (size_t)&breakpoint_handler);
	idt_install(4, (size_t)&overflow_handler);
	idt_install(5, (size_t)&bound_handler);
	idt_install(6, (size_t)&opcode_handler);
	idt_install(7, (size_t)&device_handler);
	idt_install(8, (size_t)&double_handler);
	idt_install(9, (size_t)&coprocessor_handler);
	idt_install(10, (size_t)&tss_handler);
	idt_install(11, (size_t)&segment_handler);
	idt_install(12, (size_t)&stack_handler);
	idt_install(13, (size_t)&gpf_handler);
	idt_install(14, (size_t)&page_handler);
	idt_install(15, (size_t)&reserved_handler);
	idt_install(16, (size_t)&floating_handler);
	idt_install(17, (size_t)&alignment_handler);
	idt_install(18, (size_t)&machine_handler);
	idt_install(19, (size_t)&simd_handler);
	idt_install(20, (size_t)&virtual_handler);
	idt_install(21, (size_t)&reserved_handler);
	idt_install(22, (size_t)&reserved_handler);
	idt_install(23, (size_t)&reserved_handler);
	idt_install(24, (size_t)&reserved_handler);
	idt_install(25, (size_t)&reserved_handler);
	idt_install(26, (size_t)&reserved_handler);
	idt_install(27, (size_t)&reserved_handler);
	idt_install(28, (size_t)&reserved_handler);
	idt_install(29, (size_t)&reserved_handler);
	idt_install(30, (size_t)&security_handler);
	idt_install(31, (size_t)&reserved_handler);
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

	cpu_t FS_BASE *cpu = (cpu_t FS_BASE *)0;
	kprintf("EXCEPTION ON CPU %d: %s, error code 0x%xd\n", cpu->index, string, code);
	dump_registers(&registers);

	while(1);
}




