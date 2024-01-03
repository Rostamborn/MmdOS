#ifndef KERNEL_SCHEDULER_DEF
#define KERNEL_SCHEDULER_DEF

#include "../cpu/cpu.h"
#include "process.h"

// core function of the scheduler
// TODO: BETTER ALGORITHM
// TODO: ADD PRIORITY QUEUES
// TODO: SUPPORT MULTIPLE CORES
interrupt_frame* schedule(interrupt_frame* restrict context);

void scheduler_init();

#endif
