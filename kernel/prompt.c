#include "prompt.h"
#include "limine_term.h"
#include "print.h"
#include <stdint.h>
void prompt_init() {
    for (int i = 0; i < PROMPT_BUFFER_SIZE; i++) {
        buffer[i] = '\0';
    }
    printf("$: ");
}

void prompt_enter_handler() {
    if (line_len > 0) {
        printf("\n");
        line_num++;
        printf("%s", buffer);
    }

    for (int i = 0; buffer_pointer > 0; buffer_pointer--) {
        buffer[buffer_pointer - 1] = '\0';
    }
    line_len = 0;
    line_num++;
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
    printf("\b \b");
}
