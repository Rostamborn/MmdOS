#ifndef USTAR_KERNEL_DEF
#define USTAR_KERNEL_DEF

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define MAX_USTAR_OPEN_FILES 512

typedef struct tar_header_t {
    char filename[100];
    char file_mode[8];          // octal
    char user_id[8];            // octal
    char group_id[8];           // octal
    char file_size[12];         // bytes in octal
    char last_modification[12]; // unix time in octal
    char checksum[8];
    char type_flag[1];
    char linked_file_name[100];
    char indicator[6]; // ustar then null
    char version[2];   // e.q. "00"
    char owner_username[32];
    char owner_groupname[32];
    char device_major_num[8];
    char device_minor_num[8];
    char filename_prefix[155];
} tar_header_t;

typedef struct ustar_file_descriptor {
    tar_header_t* header;
    uint16_t      references;
    ssize_t       size;
} ustar_file_descriptor;

typedef struct ustar_directory_descriptor {
    tar_header_t* header;
    uint16_t      references;
    ssize_t       size;
} ustar_directory_descriptor;

int64_t  ustar_open(char* path, int flags);
uint64_t ustar_read(uint64_t file_id, char* buffer, size_t nbytes,
                    uint64_t offset);
int64_t  ustar_close(uint64_t file_id);

uint64_t ustar_read_dir(char* path, char* buffer, size_t nbytes,
                        uint64_t offset);

void    ustar_set_start_address(uint64_t address);
ssize_t ustar_get_file_size(uint64_t file_id);
#endif