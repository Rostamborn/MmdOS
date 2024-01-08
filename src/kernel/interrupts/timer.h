#ifndef KERNEL_TIMER_DEF
#define KERNEL_TIMER_DEF

#include <stdint.h>

void timer_init();

uint64_t timer_get_uptime();

#endif
