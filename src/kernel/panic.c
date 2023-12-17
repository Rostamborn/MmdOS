#include "cpu.h"
#include "print.h"
#include <stdarg.h>

void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    kernel_printf(fmt, args);

    va_end(args);

    hcf();
}
