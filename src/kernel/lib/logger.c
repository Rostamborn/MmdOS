#include "logger.h"
#include "../cpu/cpu.h"
#include "formatter.h"
#include <stdarg.h>
#include <stdint.h>

#define LOGGER_PREFIX_LENGTH 32
char prefix[LOGGER_PREFIX_LENGTH] = {0};

void klog(const char* new_prefix, const char* fmt, ...) {
    if (PROD_MODE) {
        return;
    }
    // loading optional parameters
    va_list args;
    va_start(args, fmt);

    vklog(new_prefix, fmt, args);

    va_end(args);
}

void vklog(const char* new_prefix, const char* fmt, va_list args) {
    char buffer[MAX_STRING_FORMATTER_BUFFER_SIZE] = {0};

    if (new_prefix != 0) {
        int i = 0;
        while (*new_prefix != '\0' && i < LOGGER_PREFIX_LENGTH - 2) {
            prefix[i] = *new_prefix;
            new_prefix++;
            i++;
        }
        prefix[i] = ' ';
        i++;
        prefix[i] = '\0';
    }

    int i = format_string(prefix, buffer, 0);
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
    log_to_serial(buffer);
}
