#include "limine_terminal.h"
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
