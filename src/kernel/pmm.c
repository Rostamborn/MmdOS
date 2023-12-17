#include <stdint.h>
#include "limine.h"
#include "cpu.h"
#include "print.h"
#include "spinlock.h"
#include "util.h"
#include "panic.h"
#include "bitmap.h"

struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

spinlock_t spin_lock = SPINLOCK_INIT;
static uint8_t* bitmap = 0;
static uint64_t page_index_limit = 0;
static uint64_t prev_page_index = 0;
static uint64_t total_pages = 0;
static uint64_t usable_pages = 0;
static uint64_t used_pages = 0;
static uint64_t reserved_pages = 0;

void pmm_init() {
    struct limine_memmap_response* memmap = memmap_req.response;
    struct limine_hhdm_response* hhdm = hhdm_req.response;
    struct limine_memmap_entry** entries = memmap->entries;

    uint64_t highest_addr = 0;

    printf("pmm: Memmap entry count: %d\n", memmap->entry_count);

    for(int i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];

        /* For some reason, entry->Type seems to be NULL */
        
        printf("addr: %x, size: %d\n", entry->base, entry->length, entry->type);
        // if(entry->type != 1) continue;
        switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
                usable_pages += div_round_up(entry->length, PAGE_SIZE);
                highest_addr = max(highest_addr, entry->base + entry->length);
                break;
            case LIMINE_MEMMAP_RESERVED:
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            case LIMINE_MEMMAP_ACPI_NVS:
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
                reserved_pages += div_round_up(entry->length, PAGE_SIZE);
                break;
        }
    }

    page_index_limit = highest_addr / PAGE_SIZE;
    uint64_t bitmap_size = page_align_up(page_index_limit / 8); // becaue we use 1 bit per page
    // uint64_t bitmap_size = div_round_up(page_index_limit , 8); // becaue we use 1 bit per page

    printf("pmm: highest addr: %x\n", highest_addr);
    printf("pmm: bitmap size: %d\n", bitmap_size);
    printf("pmm: memory size: %d\n", page_index_limit * PAGE_SIZE);
    /////////////////////////
    

    // Find a hole for the bitmap in the memory map.
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length >= bitmap_size) {
            bitmap = (uint8_t *)(entry->base + hhdm->offset); // offsetting the bitmap to the higher half of the memory

            // Initialise entire bitmap to 1 (non-free)
            memset(bitmap, 0xff, bitmap_size);

            // *Not sure about this part*
            entry->length -= bitmap_size; // we occupied the usable part of this etnry
            entry->base += bitmap_size;

            break;
        }
    }

    // Populate free bitmap entries according to the memory map.
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
            bitmap_clear(bitmap, (entry->base + j) / PAGE_SIZE);
        }
    }

    printf("pmm: usable memory: %d Mib\n", (usable_pages * PAGE_SIZE) / 1024 / 1024);

}

void* physical_alloc(uint64_t n_pages, uint64_t limit) {
    uint64_t consecutive_pages = 0;

    while (prev_page_index < limit) {
        if (bitmap_get(bitmap, prev_page_index)) {
            consecutive_pages = 0;
            prev_page_index++;
        } else {
            consecutive_pages++;
            prev_page_index++;
            if (consecutive_pages == n_pages) {
                uint64_t page_index = prev_page_index - n_pages;
                for (uint64_t i = page_index; i < page_index + n_pages; i++) {
                    bitmap_set(bitmap, i);
                }
                // prev_page_index = page_index + n_pages;
                return (void*)(page_index * PAGE_SIZE); // the base addr of allocated pages
            }
        }
    }

    return NULL;
}

void* pmm_alloc(uint64_t n_pages) {
    spinlock_acquire(&spin_lock);

    uint64_t prev_index = prev_page_index;
    void* allocated = physical_alloc(n_pages, page_index_limit);

    if (allocated == NULL) { // for when we hit the page_index_limit or can't find enougth n_pages 
                             // in the current range. so we wrap to the beginning of the bitmap till
                             // the prev_page_index
        prev_page_index = 0;
        allocated = physical_alloc(n_pages, prev_index);
    }

    if (allocated != NULL) {
        memset(allocated, 0, n_pages * PAGE_SIZE);
        used_pages += n_pages;
    } else {
        panic("pmm_alloc: out of memory");
    }

    spinlock_release(&spin_lock);

    return allocated;
}

void pmm_free(void* addr, uint64_t n_pages) {
    spinlock_acquire(&spin_lock);

    uint64_t page_index = (uint64_t)addr / PAGE_SIZE;
    for (uint64_t i = page_index; i < page_index + n_pages; i++) {
        if (bitmap_get(bitmap, i)) {
            panic("pmm_free: page already free");
        }
        bitmap_clear(bitmap, i);
    }
    used_pages -= n_pages;

    spinlock_release(&spin_lock);
}
