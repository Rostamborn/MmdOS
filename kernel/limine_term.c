#include "limine_term.h"
#include "limine.h"
#include "cpu.h"
#include <stddef.h>
#include <stdint.h>

struct limine_terminal_request term_req = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

void limine_write(const char *str) {
    if (term_req.response == NULL || term_req.response->terminal_count < 1) {
        hcf();
    }

    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    // struct limine_terminal *terminal = term_req.response->terminals[0];
    term_req.response->write(term_req.response->terminals[0], str, len);
}

void limine_write_digit(uint64_t digit) {
    uint8_t dig = digit;
    uint8_t len = 0;
    while(dig > 0) {
        dig /= 10;
        len++;
    }
    dig = digit;
    char str[len + 1];
    uint8_t remainder = 0;
    for(uint8_t i = 0; i < len; i++) {
        remainder = dig % 10;
        dig /= 10;
        str[i] = remainder + '0';
    }
    str[len] = '\0'; // null termination

    // reverse string
    for(uint8_t i = 0; i < len/2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }

    limine_write(str);
}
