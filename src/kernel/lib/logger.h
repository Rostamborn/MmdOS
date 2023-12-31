#ifndef KERNEL_LOGGER_DEF
#define KERNEL_LOGGER_DEF

#include <stdarg.h>

// kernel log: load args, then call virtual kernel log (vklog)
void klog(const char* prefix, const char* fmt, ...);

// virtual kernel log: format and then log to console
void vklog(const char* prefix, const char* fmt, va_list args);

#endif
