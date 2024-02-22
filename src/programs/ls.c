#include "../kernel/fs/vfs.h"
#include "../kernel/userland/sys.h"

void ls_command() {
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
                path[index] = (char) (read_char);
                index++;
                do_syscall(4, p, 0, 0, 0);
            }
        }
    }

    char buffer[1024];
    buffer[0] = '\n';
    uint64_t read = vfs_read_dir_syscall(0, path, buffer + 1, 1023, 0);
    buffer[read] = '\0';
    do_syscall(4, buffer, 0, 0, 0);

    do_syscall(7, 0, 0, 0, 0);
}
