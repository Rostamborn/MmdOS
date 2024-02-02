#include "logger.h"
#include "src/kernel/cpu/cpu.h"
#include "src/kernel/lib/formatter.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/print.h"
#include <stdarg.h>

void panic(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[MAX_STRING_FORMATTER_BUFFER_SIZE] = {0};

    int i = 0;
    while (*fmt && i < MAX_STRING_FORMATTER_BUFFER_SIZE) {
        if (*fmt == '%') {
            fmt++;
            i = format_handler(*fmt, buffer, i, args);
            fmt++;
            continue;
        }
        buffer[i] = *fmt;
        i++;

        fmt++;
    }

    va_end(args);

    kprintf(buffer);
    klog(0, buffer);

    hcf();
}
