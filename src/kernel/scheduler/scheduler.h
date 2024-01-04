#ifndef KERNEL_SCHEDULER_DEF
#define KERNEL_SCHEDULER_DEF

#include "../cpu/cpu.h"
#include "process.h"

// core function of the scheduler
// TODO: BETTER ALGORITHM
// TODO: ADD PRIORITY QUEUES
// TODO: SUPPORT MULTIPLE CORES
execution_context* schedule(execution_context* restrict context);

void scheduler_init();

void scheduler_yield();

#endif
