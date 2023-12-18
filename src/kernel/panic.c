#include "cpu.h"
#include "print.h"
#include <stdarg.h>

void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vkprintf(fmt, args);

    va_end(args);

    hcf();
}
