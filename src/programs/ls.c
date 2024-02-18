#include "../kernel/fs/vfs.h"
#include "../kernel/userland/sys.h"

void ls_command() {
    char     buffer[1024];
    uint64_t read = vfs_read_dir_syscall(0, "/", buffer, 1024, 0);
    buffer[read] = '\0';
    do_syscall(4, buffer, 0, 0, 0);
}
