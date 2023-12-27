#include "logger.h"
#include "src/kernel/cpu/cpu.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/print.h"
#include <stdarg.h>

void panic(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vkprintf(fmt, args);
    vklog("Panic :: ", fmt, args);

    va_end(args);

    hcf();
}
