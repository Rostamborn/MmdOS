#include "vfs.h"
#include "../lib/alloc.h"
#include "../lib/print.h"
#include "../lib/util.h"
#include "../limine.h"
#include "../scheduler/process.h"
#include "ustar.h"
#include <stdbool.h>

size_t            next_mountpoint_id = 1;
mountpoint_t*     mounted_devices[MAX_MOUNTPOINTS];
uint8_t           mounted_devices_num = 0;
file_descriptor_t vfs_opened_files[MAX_OPENED_FILES];
file_descriptor_t vfs_opened_dirs[MAX_OPENED_FILES];

struct limine_module_request module_req = {.id = LIMINE_MODULE_REQUEST,
                                           .revision = 0};

int vfs_mount(char* device, char* target, char* fs_type) {
    mountpoint_t* new_mountpoint = (mountpoint_t*) kalloc(sizeof(mountpoint_t));
    fs_operations_t* operations =
        (fs_operations_t*) kalloc(sizeof(fs_operations_t));
    new_mountpoint->id = next_mountpoint_id++;
    new_mountpoint->device = device;
    kstrcpy(new_mountpoint->type, fs_type, VFS_TYPE_LENGTH);
    kstrcpy(new_mountpoint->mountpoint, target, VFS_NAME_LENGTH);

    if (kstrcmp(fs_type, "ustar", 5)) {
        operations->read = ustar_read;
        operations->open = ustar_open;
        operations->close = ustar_close;
        operations->get_file_size = ustar_get_file_size;
        operations->read_dir = ustar_read_dir;
    }

    new_mountpoint->operations = operations;

    mounted_devices[mounted_devices_num] = new_mountpoint;
    mounted_devices_num++;
}

// TODO add support for relative path
// it would be a good idea to detect relative path and convert it to absolute
// path then we can proceed with the current code
mountpoint_t* vfs_get_mountpoint(char* path) {

    u_int16_t longest = 0;
    u_int8_t  id = 0;

    for (u_int8_t i = 0; i < mounted_devices_num; i++) {
        if (mounted_devices[i] == NULL) {
            continue;
        }
        char*    c = path;
        uint16_t j = 0;
        char     mountpoint[VFS_NAME_LENGTH];
        kstrcpy(mountpoint, mounted_devices[i]->mountpoint, VFS_NAME_LENGTH);
        while (c != NULL && j < VFS_NAME_LENGTH) {
            if (*c == mountpoint[j]) {
                j++;
                c++;
            } else {
                if (j > longest) {
                    longest = j;
                    id = i;
                }
                break;
            }
        }
    }

    return mounted_devices[id];
}

mountpoint_t* get_mountpoint_by_id(size_t mountpoint_id) {
    return mounted_devices[mountpoint_id - 1];
}

char* get_rel_path(mountpoint_t* m, const char* path) {
    char* m_path = (char*) m->mountpoint;
    while ((*m_path != "\0") && (*path != "\0")) {
        if (*m_path == *path) {
            m_path++;
            path++;
        } else {
            break;
        }
    }
    return path;
}

int vfs_open(const char* path, int flags) {
    mountpoint_t* mountpoint = vfs_get_mountpoint(path);
    if (mountpoint == NULL) {
        return -1;
    }
    char* rel_path = get_rel_path(mountpoint, path);
    int   fs_specific_id = mountpoint->operations->open(rel_path, flags);
    if (fs_specific_id == -1) {
        return -1;
    }
    uint64_t vfs_id = 0;
    bool     found = false;
    for (uint64_t i = 0; i < MAX_OPENED_FILES; i++) {
        if (vfs_opened_files[i].fs_file_id == 0) {
            vfs_id = i;
            found = true;
            break;
        }
    }
    if (!found) {
        return -1;
    }
    file_descriptor_t* file_descriptor =
        (file_descriptor_t*) kalloc(sizeof(file_descriptor));

    file_descriptor->fs_file_id = fs_specific_id;
    file_descriptor->mountpoint_id = mountpoint->id;
    // TODO: file_descriptor->filename;
    file_descriptor->buf_read_pos = 0;
    file_descriptor->buf_write_pos = 0;
    file_descriptor->file_size =
        mountpoint->operations->get_file_size(fs_specific_id);
    // TODO: file_descriptor->file_buffer;
    vfs_opened_files[vfs_id] = *file_descriptor;

    return vfs_id;
}

int vfs_close(int file_descriptor_id) {
    if (vfs_opened_files[file_descriptor_id].fs_file_id == -1) {
        return 0;
    }
    int mountpoint_id = vfs_opened_files[file_descriptor_id].mountpoint_id;
    mountpoint_t* mountpoint = get_mountpoint_by_id(mountpoint_id);
    uint64_t      fs_file_id = vfs_opened_files[file_descriptor_id].fs_file_id;
    int           fs_close_result = mountpoint->operations->close(fs_file_id);
    if (fs_close_result == 0) {
        vfs_opened_files[file_descriptor_id].fs_file_id = -1;
        return 0;
    }
    return -1;
}

uint64_t vfs_read(int file_descriptor_id, void* buf, size_t nbytes) {
    file_descriptor_t* file = &vfs_opened_files[file_descriptor_id];
    if (file->fs_file_id == -1) {
        return 0;
    }
    int           mountpoint_id = file->mountpoint_id;
    mountpoint_t* mountpoint = get_mountpoint_by_id(mountpoint_id);
    uint64_t      fs_file_id = file->fs_file_id;
    uint64_t bytes_read = mountpoint->operations->read(fs_file_id, buf, nbytes,
                                                       file->buf_read_pos);

    if (bytes_read == 0) {
        return 0;
    }
    file->buf_read_pos += bytes_read;
    return bytes_read;
}

uint64_t vfs_read_dir(char* path, void* buf, size_t nbytes, uint64_t offset) {
    kprintf("");
    mountpoint_t* mountpoint = vfs_get_mountpoint(path);
    if (mountpoint == NULL) {
        return 0;
    }
    char*    rel_path = get_rel_path(mountpoint, path);
    uint64_t bytes_read =
        mountpoint->operations->read_dir(rel_path, buf, nbytes, offset);
    if (bytes_read == 0) {
        return 0;
    }

    return bytes_read;
}

ssize_t vfs_write(int file_descriptor_id, void* buf, size_t nbytes) {
    if (vfs_opened_files[file_descriptor_id].fs_file_id == -1) {
        return -1;
    }

    int mountpoint_id = vfs_opened_files[file_descriptor_id].mountpoint_id;
    mountpoint_t* mountpoint = get_mountpoint_by_id(mountpoint_id);
    int           fs_file_id = vfs_opened_files[file_descriptor_id].fs_file_id;
    int bytes_written = mountpoint->operations->write(fs_file_id, buf, nbytes);
    if (bytes_written < 0) {
        return bytes_written;
    }
    if (vfs_opened_files[file_descriptor_id].buf_write_pos + nbytes <
        vfs_opened_files[file_descriptor_id].file_size) {
        vfs_opened_files[file_descriptor_id].buf_write_pos += nbytes;
    } else {
        vfs_opened_files[file_descriptor_id].buf_write_pos += nbytes;
        vfs_opened_files[file_descriptor_id].file_size =
            vfs_opened_files[file_descriptor_id].buf_write_pos;
    }
    return bytes_written;
}

int vfs_execute(const char* path) {
    int id = vfs_open(path, 0);
    kprintf("id: %d", id);
    if (id < 0) {
        return id;
    }
    uint64_t  size = vfs_opened_files[id].file_size;
    uint64_t  read;
    uint64_t* dest = kalloc(size);
    uint64_t* iter = dest;

    do {
        uint64_t buffer[512];
        read = vfs_read(id, buffer, 512);
        kprintf("read: %u\n", read);
        for (uint64_t j = 0; j < read; j++) {
            *iter = buffer[j];
            iter++;
        }
    } while (read != 0);

    process_t* p = process_create("printer", dest, 0);
    kprintf(p->name);
}

uint64_t vfs_open_syscall(uint64_t frame, uint64_t path, uint64_t flags,
                          uint64_t unused, uint64_t unused2) {
    return vfs_open((char*) path, (int) flags);
}

uint64_t vfs_close_syscall(uint64_t frame, uint64_t file_id, uint64_t unused1,
                           uint64_t unused2, uint64_t unused3) {
    return vfs_close((int) file_id);
}

uint64_t vfs_read_syscall(uint64_t frame, uint64_t file_id, uint64_t buf,
                          uint64_t nbytes, uint64_t unused) {
    return vfs_read((int) file_id, (void*) buf, (size_t) nbytes);
}

uint64_t vfs_read_dir_syscall(uint64_t frame, uint64_t path, uint64_t buf,
                              uint64_t nbytes, uint64_t offset) {
    return vfs_read_dir((char*) path, (void*) buf, (size_t) nbytes, offset);
}

void vfs_init() {
    ustar_set_start_address(module_req.response->modules[0]->address);
    vfs_mount("main memory", "/", "ustar");
}
