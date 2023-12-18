#ifndef KERNEL_PRINTF_DEF
#define KERNEL_PRINTF_DEF

#include <stdarg.h>

// load args, then format and print
void printf(const char *fmt, ...);

// format and print
void kernel_printf(const char *fmt, va_list args);

#endif
