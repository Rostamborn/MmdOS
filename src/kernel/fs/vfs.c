#include "vfs.h"
#include "../lib/alloc.h"
#include "../lib/util.h"
#include <stdbool.h>

int vfs_mount(char* device, char* target, char* fs_type) {
    mountpoint_t* new_mountpoint = (mountpoint_t*) kalloc(sizeof(mountpoint_t));
    new_mountpoint->id = next_mountpoint_id++;
    new_mountpoint->device = device;
    kstrcpy(new_mountpoint->type, fs_type, VFS_TYPE_LENGTH);
    new_mountpoint->mountpoint[mounted_devices_num] = target;
    new_mountpoint->operations = NULL;

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
        char     c = *path;
        uint16_t j = 0;
        char     mountpoint[] = mounted_devices[i]->mountpoint;
        while (c && j < VFS_NAME_LENGTH) {
            if (c == mountpoint[j]) {
                j++;
                c++;
            } else {
                if (j > longest) {
                    longest = j;
                    id = i;
                    break;
                }
            }
        }
    }

    return mounted_devices[id];
}

mountpoint_t* get_mountpoint_by_id(size_t mountpoint_id) {
    for (int i = 0; i < MAX_MOUNTPOINTS; i++) {
        if (mounted_devices[i]->id == mountpoint_id) {
            return mounted_devices[i];
        }
    }
    return NULL;
}

char* get_rel_path(mountpoint_t* m, const char* path) {
    while (m && path) {
        if (m == path) {
            m++;
            path++;
        } else {
            break;
        }
    }
    return path;
}

int vfs_open(const char* path, int flags) {
    mountpoint_t* mountpoint = vf_get_mountpoint(path);
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
        if (vfs_opened_files[i].fs_file_id == -1) {
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
    // TODO: file_descriptor->file_size;
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

ssize_t vfs_read(int file_descriptor_id, void* buf, size_t nbytes) {

    if (vfs_opened_files[file_descriptor_id].fs_file_id == -1) {
        return -1;
    }

    int mountpoint_id = vfs_opened_files[file_descriptor_id].mountpoint_id;
    mountpoint_t* mountpoint = get_mountpoint_by_id(mountpoint_id);
    int           fs_file_id = vfs_opened_files[file_descriptor_id].fs_file_id;
    int bytes_read = mountpoint->operations->read(fs_file_id, buf, nbytes);
    if (bytes_read < 0) {
        return bytes_read;
    }
    if (vfs_opened_files[file_descriptor_id].buf_read_pos + nbytes <
        vfs_opened_files[file_descriptor_id].file_size) {
        vfs_opened_files[file_descriptor_id].buf_read_pos += nbytes;
    } else {
        vfs_opened_files[file_descriptor_id].buf_read_pos =
            vfs_opened_files[file_descriptor_id].file_size;
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