#include "../kernel/lib/print.h"
#include "../kernel/userland/sys.h"

void cat_command(char* path) {
    char* p = path;
    int   id = do_syscall(1, p, 0, 0, 0);
    if (id < 0) {
        do_syscall(4, "file could not be open!\n", 0, 0, 0);
        do_syscall(4, path, 0, 0, 0);
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
}