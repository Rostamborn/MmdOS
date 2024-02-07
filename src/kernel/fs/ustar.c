#include "ustar.h"
#include "../lib/alloc.h"
#include "../lib/print.h"
#include "../lib/util.h"

uint64_t                    tar_fs_start_address;
ustar_file_descriptor*      open_files[MAX_USTAR_OPEN_FILES] = {0};
ustar_directory_descriptor* open_dirs[MAX_USTAR_OPEN_FILES] = {0};

int64_t is_already_open(char* name) {
    for (int64_t i = 0; i < MAX_USTAR_OPEN_FILES; i++) {
        if (open_files[i] != NULL && (open_files[i]->references > 0)) {
            char tar_filename[256] = {0};
            if (open_files[i]->header->filename_prefix[0] != 0) {
                kstrcpy(tar_filename, open_files[i]->header->filename_prefix,
                        155);
                kstrcpy(tar_filename + 155, open_files[i]->header->filename,
                        100);
            } else {
                kstrcpy(tar_filename, open_files[i]->header->filename_prefix,
                        155);
            }

            if (kstrcmp(tar_filename, name, 256)) {
                return i;
            }
        }
    }
    return -1;
}

int64_t find_open_id() {
    for (int64_t i = 0; i < MAX_USTAR_OPEN_FILES; i++) {
        if (open_files[i] == NULL) {
            return i;
        }
    }
    return -1;
}

bool is_zeroed(char* record) {
    for (uint16_t i = 0; i < 512; i++) {
        if (*record != 0) {
            return false;
        }
        record++;
    }
    return true;
}

tar_header_t* tar_file_lookup(const char* searched_file) {
    char tar_filename[256];
    int  zero_counter = 0;
    // The starting address should be known somehow to the OS
    tar_header_t* current_record = (tar_header_t*) tar_fs_start_address;
    while (zero_counter < 2) {

        if (is_zeroed(current_record)) {
            zero_counter++;
            continue;
        }
        zero_counter = 0;
        if (current_record->filename_prefix[0] != 0) {
            kstrcpy(tar_filename, current_record->filename_prefix, 155);
            kstrcpy(tar_filename + 155, current_record->filename, 100);
        } else {
            kstrcpy(tar_filename, (char*) (current_record->filename), 155);
        }
        for (int i = 0; i < 255; i++) {
            if (tar_filename[i] == '\\') {
                tar_filename[i] = '/';
            }
        }
        if (kstrcmp((char*) tar_filename, searched_file, 256)) {
            return current_record;
        }

        // move to next record
        uint64_t file_size = oct_ascii_to_dec(current_record->file_size, 12);
        if (file_size % 512 != 0) {
            file_size = (file_size / 512 + 1) * 512;
        }
        current_record =
            (tar_header_t*) ((uint64_t) (current_record) + 512 + file_size);
    }

    // file not found
    return NULL;
}

tar_header_t* tar_file_prefix_lookup(const char*   searched_directory,
                                     uint64_t      length,
                                     tar_header_t* current_record) {
    char tar_filename[256];
    int  zero_counter = 0;
    // The starting address should be known somehow to the OS
    if (current_record == NULL) {
        current_record = (tar_header_t*) tar_fs_start_address;
    }
    while (zero_counter < 2) {

        if (is_zeroed(current_record)) {
            zero_counter++;
            continue;
        }
        zero_counter = 0;
        if (current_record->filename_prefix[0] != 0) {
            kstrcpy(tar_filename, current_record->filename_prefix, 155);
            kstrcpy(tar_filename + 155, current_record->filename, 100);
        } else {
            kstrcpy(tar_filename, (char*) (current_record->filename), 155);
        }
        for (int i = 0; i < 255; i++) {
            if (tar_filename[i] == '\\') {
                tar_filename[i] = '/';
            }
        }
        if (kstrcmp((char*) tar_filename, searched_directory, length)) {
            return current_record;
        }

        // move to next record
        uint64_t file_size = oct_ascii_to_dec(current_record->file_size, 12);
        if (file_size % 512 != 0) {
            file_size = (file_size / 512 + 1) * 512;
        }
        current_record =
            (tar_header_t*) ((uint64_t) (current_record) + 512 + file_size);
    }

    // file not found
    return NULL;
}
int64_t ustar_open(char* path, int flags) {
    int64_t id = is_already_open(path);
    if (id > -1) {
        open_files[id]->references++;
        return id;
    }
    tar_header_t* header = tar_file_lookup(path);
    if (header == NULL) {
        return -1;
    }
    ustar_file_descriptor* file_descriptor =
        (ustar_file_descriptor*) kalloc(sizeof(ustar_file_descriptor));

    id = find_open_id();
    if (id == -1) {
        return -1;
    }

    file_descriptor->header = header;
    file_descriptor->references = 1;
    file_descriptor->size = oct_ascii_to_dec(header->file_size, 12);
    open_files[id] = file_descriptor;
    return id;
}

uint64_t ustar_read(uint64_t file_id, char* buffer, uint64_t nbytes,
                    uint64_t offset) {
    char* b = buffer;

    ustar_file_descriptor* file = open_files[file_id];

    if (file == NULL) {
        return 0;
    }

    if (offset > file->size) {
        return 0;
    }

    char* content_pointer = (char*) ((uint64_t) file->header + 512 + offset);

    uint64_t i;
    for (i = 0; (i < nbytes); i++) {
        *b = *content_pointer;
        b++;
        content_pointer++;
    }
    return i;
}

uint64_t ustar_read_dir(char* dir, char* buffer, uint64_t nbytes,
                        uint64_t offset) {
    char* b = buffer;

    uint64_t length = 0;
    char*    iterator = dir;

    while (*iterator != 0) {
        length++;
        iterator++;
    }

    tar_header_t* header = NULL;
    while (true) {
        header = tar_file_prefix_lookup(dir, length, header);
        }
}

int64_t ustar_close(uint64_t file_id) {
    open_files[file_id]->references--;
    if (open_files[file_id]->references < 1) {
        kfree(open_files[file_id]);
        open_files[file_id] = NULL;
    }
    return 0;
}

void ustar_set_start_address(uint64_t address) {
    tar_fs_start_address = address;
}

ssize_t ustar_get_file_size(uint64_t file_id) {
    return open_files[file_id]->size;
}