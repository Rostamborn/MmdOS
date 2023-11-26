#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "acpi.h"
#include "cpu.h"

static struct limine_rsdp_request rsdp_req = { // Root System Description Pointer
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

struct rsdp { // root system description pointer
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extented_checksum;
    uint8_t reserved[3];
};

struct sdt_header { // system description table header(header entries of rsdt/xsdt)
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
};

struct rsdt { // root system description table
    struct sdt_header header;
    char sdt_entry[];
} __attribute__((packed));

struct madt { // multiple APIC description table
    struct sdt_header header;
    uint32_t local_controller_addr;
    uint32_t flags;
    // char madt_entries[];
} __attribute__((packed));

void acpi_init() {
    struct limine_rsdp_response *rsdp_resp = rsdp_req.response;
    if (rsdp_resp == NULL || rsdp_resp->address == NULL) {
        hcf();
    }

    // struct rsdp *rsdp = rsdp_resp->address;

    // TODO: I don't know what to do D:
}
