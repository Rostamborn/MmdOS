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

struct rsdt { // root system description table
    struct sdt_header header;
    char sdt_entry[];
} __attribute__((packed));

// struct madt { // multiple APIC description table
//     struct sdt_header header;
//     uint32_t local_controller_addr;
//     uint32_t flags;
//     // char madt_entries[];
// } __attribute__((packed));

void acpi_init() {
    struct limine_rsdp_response *rsdp_resp = rsdp_req.response;
    if (rsdp_resp == NULL || rsdp_resp->address == NULL) {
        hcf();
    }

    rsdp = rsdp_resp->address;


    //! -------------

    if (use_xsdt()) {
        rsdt = (struct rsdt *)(rsdp->xsdt_addr + VMM_HIGHER_HALF);
    } else {
        rsdt = (struct rsdt *)((uint64_t)rsdp->rsdt_addr + VMM_HIGHER_HALF);
    }

    kernel_print("acpi: Revision: %lu\n", rsdp->revision);
    kernel_print("acpi: Uses XSDT? %s\n", use_xsdt() ? "true" : "false");
    kernel_print("acpi: RSDT at %lx\n", rsdt);

    struct sdt *fadt = acpi_find_sdt("FACP", 0);
    if (fadt != NULL && fadt->length >= 116) {
        uint32_t fadt_flags = *((uint32_t *)fadt + 28);

        if ((fadt_flags & (1 << 20)) != 0) {
            panic(NULL, true, "Lyre does not support HW reduced ACPI systems");
        }
    }

    madt_init();
}
