#ifndef STRING_FORMATTER_DEF
#define STRING_FORMATTER_DEF

#include <stdarg.h>

#define MAX_STRING_FORMATTER_BUFFER_SIZE 1024
#define MAX_HEX_HANDLER_BUFFER_SIZE 64

int format_handler(char format, char buffer[], int buffer_offset, va_list args);
int format_string(char *s, char buffer[], int buffer_offset);
#endif