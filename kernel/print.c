#include <stdarg.h>
#include "limine_term.h"

int format_string(char *s, char buffer[], int buffer_offset) {
    while (*s) {
        buffer[buffer_offset] = *s;
        buffer_offset++;
        s++;
    }
    return buffer_offset;
}

int format_char(int c, char buffer[], int buffer_offset) {
    buffer[buffer_offset] = c;
    buffer_offset++;
    return buffer_offset;
}

int format_int(int integer, char buffer[], int buffer_offset) {
    int len = 0;
    int remainder = integer;
    while(remainder != 0) {
        len++;
        remainder /= 10;
    }

    char number[len];

    for(int i = 0; i < len; i++) {
        number[i] = (integer % 10) + '0';
        integer /= 10;
    }

    for(int i = len-1; i >= 0; i--) {
       
        buffer[buffer_offset] = number[i];
        buffer_offset++;
    }

    return buffer_offset;
}

int format_handle(char format, char buffer[], int buffer_offset, va_list args) {
    switch (format) {
        // digit/int
        case 'd':
        case 'i': 
            int int_arg = va_arg(args, int) ;
            buffer_offset = format_int(int_arg, buffer, buffer_offset);
            break;
        

        // unsigned int
        case 'u':
            break;

        // unsigned octal
        case 'o':
            break;

        // unsigned hexadecimal
        case 'X':
            // parse to lowercase
        case 'x':
            break;

        // decimal floating point
        case 'F':
        case 'f':
            break;
        

        // scietific notation
        case 'E':
        case 'e':
            break;


        // I don't even know
        case 'G':
        case 'g':
            break;


        // hexadecimal floating point
        case 'A':
        case 'a':
            break;

        // char
        case 'c':
            int arg_char = va_arg(args, int) ;
            buffer_offset = format_char(arg_char, buffer, buffer_offset);
            break;

        // string
        case 's':
            char* arg_str = va_arg(args, char*) ;
            buffer_offset = format_string(arg_str, buffer, buffer_offset);
            break;
        default:
            break;
    }
    return buffer_offset;
}

void kernel_printf(const char *fmt, ... ) {
    // loading optional parameters
    va_list args;
    va_start(args, fmt);

    char buffer[1024];
    
    int i = 0;
    while(*fmt && i < 1024) {
        if(*fmt == '%') {
            fmt++;
            i = format_handle(*fmt, buffer, i, args);
            fmt++;
            continue;
        }
        buffer[i] = *fmt;
        i++;

        fmt++;
    }
    limine_write(buffer);

    va_end(args);
}
