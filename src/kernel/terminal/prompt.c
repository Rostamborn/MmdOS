#include "prompt.h"
#include "../../programs/cat.h"
#include "../../programs/gameoflife/gameoflife.h"
#include "../../programs/snake/snake.h"
#include "../lib/print.h"
#include "../lib/spinlock.h"
#include "../lib/util.h"
#include "../scheduler/process.h"
#include "limine_term.h"
#include <stdint.h>

#define PROMPT_BUFFER_SIZE 1024

uint8_t  line_len = 0;
char     buffer[PROMPT_BUFFER_SIZE];
uint16_t buffer_pointer = 0;
uint8_t  line_num = 1;

bool   stdin_lock = false;
size_t locker_id = 0;

uint8_t prompt_lockstdin(size_t caller_id) {
    spinlock_t lock = SPINLOCK_INIT;
    spinlock_acquire(&lock);
    if (!stdin_lock) {
        locker_id = caller_id;
        stdin_lock = true;
        spinlock_release(&lock);
        return 1;
    }
    spinlock_release(&lock);
    return 0;
}

void prompt_unlockstdin(size_t caller_id) {
    spinlock_t lock = SPINLOCK_INIT;
    spinlock_acquire(&lock);
    if (stdin_lock && locker_id == caller_id) {
        locker_id = 0;
        stdin_lock = false;
    }
    spinlock_release(&lock);
}

uint64_t prompt_lockstdin_syscall(uint64_t frame, uint64_t unused1,
                                  uint64_t unused2, uint64_t unused3,
                                  uint64_t unused4) {
    process_t* p = process_get_current();
    uint64_t   result = prompt_lockstdin(p->running_thread->tid);
    return result;
}

uint64_t prompt_unlockstdin_syscall(uint64_t frame, uint64_t unused1,
                                    uint64_t unused2, uint64_t unused3,
                                    uint64_t unused4) {
    process_t* p = process_get_current();
    prompt_unlockstdin(p->running_thread->tid);
    kprintf("\n$: ");
    return 0;
}

void prompt_process() {
    for (int i = 0; i < PROMPT_BUFFER_SIZE; i++) {
        buffer[i] = '\0';
    }
    while (true) {
        if (stdin_lock) {
            continue;
        }
        // keyboard_getch syscall
        uint64_t call_result = do_syscall(5, 0, 0, 0, 0);

        if (call_result != (uint64_t) (-1)) {
            prompt_handler((char) call_result);
        }

        // TODO: yield and block syscall
    }
}

void prompt_init() {
    process_create("prompt", prompt_process, 0);
    kprintf("$: ");
}

void prompt_handler(char c) {
    switch (c) {
    case '\b':
        prompt_backspace_handler();
        break;
    case '\n':
        prompt_enter_handler();
        break;

    default:
        prompt_char_handler(c);
        break;
    }
}

void prompt_enter_handler() {
    kprintf("\n");
    bool yield = false;
    if (line_len > 0) {
        line_num++;
        if (kstrcmp(buffer, "clear", 5)) {
            prompt_clear();

        } else if (kstrcmp(buffer, "cat", 3)) {
            process_create("cat", cat_command, (char*) buffer);
            yield = true;
        } else if (kstrcmp(buffer, "gol", 3)) {
            process_create("gol", gol_game_loop, 3);
            yield = true;
        } else if (kstrcmp(buffer, "snake", 5)) {
            process_create("gol", snake_game_loop, 3);
            yield = true;
        } else {
            kprintf("%s\n", buffer);
        }
    }

    for (int i = 0; buffer_pointer > 0; buffer_pointer--) {
        buffer[buffer_pointer - 1] = '\0';
    }
    line_len = 0;
    line_num++;
    if (yield == true) {
        // TODO: yield and block syscall
    } else {
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
    kprintf("%c", c);
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
    char backspace[143] = {[0 ... 141] = '\b'};
    char space[143] = {[0 ... 100] = ' '}; // Arman: 100 char, Mahbod 141 char
    for (int i = 48; i > -1; i--) {
        kprintf("\033[F%s", space);
    }
    kprintf("\033[F");
    line_num = 1;
    line_len = 0;
}
