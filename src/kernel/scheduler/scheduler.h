#ifndef KERNEL_SCHEDULER_DEF
#define KERNEL_SCHEDULER_DEF

#include "src/kernel/cpu/cpu.h"
#include "src/kernel/scheduler/process.h"

// core function of the scheduler
// TODO: BETTER ALGORITHM
// TODO: ADD PRIORITY QUEUES
// TODO: SUPPORT MULTIPLE CORES
interrupt_frame* schedule(interrupt_frame* context);

void scheduler_init();

void add_process(process_t* process);

#endif