#ifndef KERNEL_PANIC_DEF
#define KERNEL_PANIC_DEF

#include <stdarg.h>

void panic(const char *fmt, ...);

#endif