#ifndef KERNEL_PRINTF_DEF
#define KERNEL_PRINTF_DEF

#include <stdarg.h>
#include <stdint.h>

// kernel printf: load args, then call virtual kernel printf (vkprintf)
void kprintf(const char* fmt, ...);

// virtual kernel printf: format and print
void vkprintf(const char* fmt, va_list args);

uint64_t print_syscall(uint64_t frame, uint64_t str, uint64_t unused,
                       uint64_t unused2, uint64_t unused3);
#endif
