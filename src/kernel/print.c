#include "print.h"
#include "formatter.h"
#include "limine_term.h"
#include "stdint.h"
#include <stdarg.h>

void printf(const char *fmt, ...) {
    // loading optional parameters
    va_list args;
    va_start(args, fmt);

    kernel_printf(fmt, args);

    va_end(args);
}

void kernel_printf(const char *fmt, va_list args) {
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
    limine_write(buffer);
}
