#include "formatter.h"
#include <stdint.h>

void int_to_hex(uint64_t i, char *buffer) {
    char hex_chars[16] = "0123456789abcdef";
    uint64_t num = i;
    uint8_t rem = 0;

    uint64_t index = 0;

    while (num != 0 && index < MAX_HEX_HANDLER_BUFFER_SIZE) {
        rem = num % 16;
        num /= 16;
        buffer[index] = hex_chars[rem];
        index++;
    }

    uint32_t k = 0;
    uint32_t j = index - 1;
    while (k < j) {
        char temp = buffer[k];
        buffer[k] = buffer[j];
        buffer[j] = temp;
        k++;
        j--;
    }

    return;
}

int format_string(char *s, char buffer[], int buffer_offset) {
    while (*s && buffer_offset < MAX_STRING_FORMATTER_BUFFER_SIZE) {
        buffer[buffer_offset] = *s;
        buffer_offset++;
        s++;
    }
    return buffer_offset;
}

int format_char(int c, char buffer[], int buffer_offset) {
    if (buffer_offset < MAX_STRING_FORMATTER_BUFFER_SIZE) {
        buffer[buffer_offset] = c;
        buffer_offset++;
    }
    return buffer_offset;
}

int format_int(int64_t integer, char buffer[], int buffer_offset) {
    uint32_t len = 0;
    int64_t remainder = integer;
    while (remainder != 0) {
        len++;
        remainder /= 10;
    }

    char number[len];

    for (uint32_t i = 0; i < len; i++) {
        number[i] = (integer % 10) + '0';
        integer /= 10;
    }

    for (int i = len - 1; i >= 0; i--) {
        if (buffer_offset < MAX_STRING_FORMATTER_BUFFER_SIZE) {
            buffer[buffer_offset] = number[i];
            buffer_offset++;
        } else {
            break;
        }
    }

    return buffer_offset;
}

int format_hex(uint64_t hex_int, char buffer[], int buffer_offset) {
    char hex_string[MAX_HEX_HANDLER_BUFFER_SIZE] = {0};
    int_to_hex(hex_int, hex_string);
    return format_string(hex_string, buffer, buffer_offset);
}

int format_handler(char format, char buffer[], int buffer_offset,
                   va_list args) {
    switch (format) {
    // digit/int
    case 'd':
    case 'i':
        int64_t int_arg = va_arg(args, int64_t);
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
    case 'p':
    case 'x':
        if (buffer_offset + 2 < MAX_STRING_FORMATTER_BUFFER_SIZE) {
            buffer[buffer_offset] = '0';
            buffer[buffer_offset + 1] = 'x';
            buffer_offset += 2;
            uint64_t hex_arg = va_arg(args, uint64_t);
            buffer_offset = format_hex(hex_arg, buffer, buffer_offset);
        }
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
        int arg_char = va_arg(args, int);
        buffer_offset = format_char(arg_char, buffer, buffer_offset);
        break;

    // string
    case 's':
        char *arg_str = va_arg(args, char *);
        buffer_offset = format_string(arg_str, buffer, buffer_offset);
        break;
    default:
        break;
    }
    return buffer_offset;
}
