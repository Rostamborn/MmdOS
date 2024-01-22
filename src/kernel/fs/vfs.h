#ifndef KERNEL_VFS_DEF
#define KERNEL_VFS_DEF

#include <sys/types.h>

#define VFS_TYPE_LENGTH 32
#define VFS_NAME_LENGTH 512
#define MAX_MOUNTPOINTS 12
#define MAX_OPENED_FILES 512

// TODO: implement mountpoints using tree
struct fs_operations_t {
    int (*open)(const char* path, int flags, ...);
    int (*close)(int file_descriptor);
    ssize_t (*read)(int file_descriptor, char* read_buffer, size_t nbyte);
    ssize_t (*write)(int file_descriptor, const void* write_buffer,
                     size_t nbyte);
};
typedef fs_operations_t fs_operations_t;

typedef struct mountpoint_t {
    u_int8_t        id;
    char            type[VFS_TYPE_LENGTH];
    char            mountpoint[VFS_NAME_LENGTH];
    char*           device;
    fs_operations_t operations;
} mountpoint_t;

typedef struct file_descriptor_t {
    uint64_t fs_file_id;
    int      mountpoint_id;
    char*    filename;
    int      buf_read_pos;
    int      buf_write_pos;
    int      file_size;
    char*    file_buffer;
} file_descriptor_t;

size_t            next_mountpoint_id = 1;
mountpoint_t*     mounted_devices[MAX_MOUNTPOINTS];
uint8_t           mounted_devices_num;
file_descriptor_t vfs_opened_files[MAX_OPENED_FILES];

int           vfs_mount(char* device, char* target, char* fs_type);
int           vfs_umount(char* device, char* target);
mountpoint_t* vfs_get_mountpoint(char* path);
int           vfs_open(const char* filename, int flags);
int           vfs_close(int file_descriptor_id);
ssize_t       vfs_read(int file_descriptor_id, void* buf, size_t nbyte);
#endif