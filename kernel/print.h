#ifndef KERNEL_PRINTF_DEF
#define KERNEL_PRINTF_DEF

#include <stdarg.h>

#define MAX_PRINTF_BUFFER_SIZE 1024

// load args, then format and print
void printf(const char *fmt, ...);

// format and print
void kernel_printf(const char *fmt, va_list args);

#endif