#include "../kernel/lib/print.h"
#include "../kernel/userland/sys.h"

void cat_command() {
    char     path[512];
    uint16_t index = 0;
    uint64_t read_char;

    // lock stdin
    uint64_t success;

    while (1) {
        success = do_syscall(6, 0, 0, 0, 0);

        if (success) {
            break;
        } else {
            // resource busy, yield and block
        }
    }

    while (1) {
        read_char = do_syscall(5, 0, 0, 0, 0);
        if (read_char != (uint64_t) (-1)) {
            if (read_char == '\n') {
                break;
            } else if (read_char == '\b' && index > 0) {
                index--;
                path[index] = '\0';
                do_syscall(4, "\b \b", 0, 0, 0);
            } else {
                char p[2] = {read_char, 0};
                do_syscall(4, p, 0, 0, 0);
            }

            path[index] = (char) (read_char);
            index++;
        }
    }
    do_syscall(4, "\n", 0, 0, 0);
    int id = do_syscall(1, path, 0, 0, 0);
    if (id < 0) {
        do_syscall(4, "file could not be open!\n$: ", 0, 0, 0);
        do_syscall(7, 0, 0, 0, 0);
        return;
    }
    uint64_t read;
    do {
        char buffer[512];
        read = do_syscall(3, id, buffer, 511, 0);
        buffer[read] = '\0';

        do_syscall(4, buffer, 0, 0, 0);
    } while (read != 0);
    do_syscall(2, id, 0, 0, 0);
    do_syscall(4, "\n$: ", 0, 0, 0);
    do_syscall(7, 0, 0, 0, 0);
}