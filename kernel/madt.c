#include <stddef.h>
#include <stdint.h>
#include "cpu.h"
#include "lib/vector.h"
#include <acpi/acpi.k.h>
#include <acpi/madt.k.h>

typeof(madt_lapics) madt_lapics = (typeof(madt_lapics))VECTOR_INIT;
typeof(madt_io_apics) madt_io_apics = (typeof(madt_io_apics))VECTOR_INIT;
typeof(madt_isos) madt_isos = (typeof(madt_isos))VECTOR_INIT;
typeof(madt_nmis) madt_nmis = (typeof(madt_nmis))VECTOR_INIT;

struct madt {
    struct sdt;
    uint32_t local_controller_addr;
    uint32_t flags;
    char entries_data[];
};

void madt_init(void) {
    struct madt *madt = acpi_find_sdt("APIC", 0);
    if (madt == NULL) {
        log_to_serial("System does not have an MADT");
    }

    size_t offset = 0;
    for (;;) {
        if (madt->length - sizeof(struct madt) - offset < 2) {
            break;
        }

        struct madt_header *header = (struct madt_header *)(madt->entries_data + offset);
        switch (header->id) {
            case 0:
                log_to_serial("madt: Found local APIC #%lu\n");
                VECTOR_PUSH_BACK(&madt_lapics, (struct madt_lapic *)header);
                break;
            case 1:
                log_to_serial("madt: Found IO APIC #%lu\n");
                VECTOR_PUSH_BACK(&madt_io_apics, (struct madt_io_apic *)header);
                break;
            case 2:
                log_to_serial("madt: Found ISO #%lu\n");
                VECTOR_PUSH_BACK(&madt_isos, (struct madt_iso *)header);
                break;
            case 4:
                log_to_serial("madt: Found NMI #%lu\n");
                VECTOR_PUSH_BACK(&madt_nmis, (struct madt_nmi *)header);
                break;
        }

        offset += header->length > 2 ? header->length : 2
    }
}