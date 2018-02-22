
/*
 * lux OS kernel
 * copyright (c) 2018 by Omar Mohammad
 */

#include <tasking.h>
#include <cpu.h>
#include <mm.h>
#include <string.h>
#include <kprintf.h>

process_t *processes;

// tasking_init(): Initializes the scheduler
// Param:	Nothing
// Return:	Nothing

void tasking_init()
{
	kprintf("tasking: initializing scheduler...\n");
	processes = kcalloc(sizeof(process_t), MAX_PROCESSES);

	// configure the kernel task
	processes[0].flags = PROCESS_FLAGS_PRESENT;
	processes[0].path[0] = '/';
	processes[0].path[1] = 0;
}

// get_path(): Returns the path of the current process
// Param:	char *destination - pointer of where to store the path
// Return:	char * - pointer to destination

char *get_path(char *destination)
{
	// CPU-specific information
	cpu_t FS_BASE *cpu = (cpu_t FS_BASE*)0;
	return strcpy(destination, processes[cpu->current_pid].path);
}

// get_pid(): Returns the current PID
// Param:	Nothing
// Return:	pid_t - PID of the current process

pid_t get_pid()
{
	cpu_t FS_BASE *cpu = (cpu_t FS_BASE*)0;
	return cpu->current_pid;
}







