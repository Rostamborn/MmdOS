#include "src/kernel/terminal/prompt.h"
#include "../../programs/cat.h"
#include "../lib/util.h"
#include "../scheduler/process.h"
#include "src/kernel/lib/print.h"
#include "src/kernel/terminal/limine_term.h"
#include <stdint.h>

#define PROMPT_BUFFER_SIZE 1024

uint8_t  line_len = 0;
char     buffer[PROMPT_BUFFER_SIZE];
uint16_t buffer_pointer = 0;
uint8_t  line_num = 1;

void prompt_init() {
    for (int i = 0; i < PROMPT_BUFFER_SIZE; i++) {
        buffer[i] = '\0';
    }
    kprintf("$: ");
}

void prompt_enter_handler() {
    kprintf("\n");
    bool flag = true;
    if (line_len > 0) {
        line_num++;
        if (kstrcmp(buffer, "clear", 5)) {
            prompt_clear();

        } else if (kstrcmp(buffer, "cat ", 4)) {
            process_create("cat", cat_command, (char*) buffer);
            flag = false;
        } else {
            kprintf("%s\n", buffer);
        }
    }

    for (int i = 0; buffer_pointer > 0; buffer_pointer--) {
        buffer[buffer_pointer - 1] = '\0';
    }
    line_len = 0;
    line_num++;
    if (flag == true) {
        kprintf("$: ");
    }
}

void prompt_char_handler(char c) {
    if (c == '\n') {
        return;
    }
    line_len++;
    buffer[buffer_pointer] = c;
    buffer_pointer++;
}

void prompt_backspace_handler() {
    if (line_len == 0)
        return;
    line_len--;
    buffer_pointer--;
    buffer[buffer_pointer] = '\0';
    kprintf("\b \b");
}

void prompt_clear() {
    for (int i = (2 * line_num); i > -1; i--) {
        char backspace[143] = {[0 ... 141] = '\b'};
        char space[143] = {[0 ... 141] = ' '};

        kprintf("\033[F%s%s", backspace, space);
    }
    kprintf("\033[F");
    line_num = 1;
    line_len = 0;
}