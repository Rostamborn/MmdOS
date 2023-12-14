#include <stdint.h>
#include "limine.h"
#include "cpu.h"
#include "src/lib/print.h"

struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

void pmm_init() {
    struct limine_memmap_response* memmap = memmap_req.response;
    struct limine_memmap_entry** entries = memmap->entries;

    for(int i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];

        if(entry->type != 1) continue;

        uint64_t addr = entry->base;
        uint64_t size = entry->length;
        uint64_t end = addr + size;
        printf("addr: %d, size: %d, end: %d\n", addr, size, end);
        // while(addr < end) {
        //     uint64_t page = addr & ~(PAGE_SIZE - 1);
        //     pmm_free(page);
        //     addr += PAGE_SIZE;
        // }
    }
}
