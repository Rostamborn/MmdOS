#ifndef KERNEL_PRINTF_DEF
#define KERNEL_PRINTF_DEF

#include <stdarg.h>

// kernel printf: load args, then call virtual kernel printf (vkprintf)
void kprintf(const char *fmt, ...);

// virtual kernel printf: format and print
void vkprintf(const char *fmt, va_list args);

#endif
