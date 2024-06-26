#ifndef KERNEL_VFS_DEF
#define KERNEL_VFS_DEF

#include <stdint.h>
#include <sys/types.h>

#define VFS_TYPE_LENGTH 32
#define VFS_NAME_LENGTH 512
#define MAX_MOUNTPOINTS 12
#define MAX_OPENED_FILES 512

// TODO: implement mountpoints using tree
typedef struct fs_operations_t {
    int64_t (*open)(const char* path, int flags, ...);
    int64_t (*close)(uint64_t file_descriptor);
    uint64_t (*read)(uint64_t file_descriptor, char* read_buffer,
                     uint64_t nbyte, uint64_t offset);
    ssize_t (*write)(uint64_t file_descriptor, const void* write_buffer,
                     size_t nbyte);
    ssize_t (*get_file_size)(
        uint64_t file_descriptor); // TODO: maybe do this in open

    int64_t (*open_dir)(const char* path, int flags, ...);

    int64_t (*close_dir)(uint64_t file_descriptor);

    uint64_t (*read_dir)(uint64_t file_descriptor, char* read_buffer,
                         uint64_t nbyte, uint64_t offset);
} fs_operations_t;

typedef struct mountpoint_t {
    uint8_t         id;
    char             type[VFS_TYPE_LENGTH];
    char             mountpoint[VFS_NAME_LENGTH];
    char*            device;
    fs_operations_t* operations;
} mountpoint_t;

typedef struct file_descriptor_t {
    uint64_t fs_file_id;
    int      mountpoint_id;
    char*    filename;
    uint64_t buf_read_pos;
    uint64_t buf_write_pos;
    uint64_t file_size;
    char*    file_buffer;
} file_descriptor_t;

int vfs_mount(char* device, char* target, char* fs_type);
// int           vfs_umount(char* device, char* target);
// mountpoint_t* vfs_get_mountpoint(char* path);
uint64_t vfs_open_syscall(uint64_t frame, uint64_t filename, uint64_t flags,
                          uint64_t unused, uint64_t unused2);
uint64_t vfs_close_syscall(uint64_t frame, uint64_t file_descriptor_id,
                           uint64_t unused, uint64_t unused2, uint64_t unused3);
uint64_t vfs_read_syscall(uint64_t frame, uint64_t file_id, uint64_t buf,
                          uint64_t nbyte, uint64_t unused);

uint64_t vfs_read_dir_syscall(uint64_t frame, uint64_t path, uint64_t buf,
                              uint64_t nbytes, uint64_t offset);
void     vfs_init();

int vfs_execute(const char* path);

#endif
