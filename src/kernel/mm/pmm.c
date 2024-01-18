#include "pmm.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../lib/spinlock.h"
#include "../lib/util.h"
#include "../limine.h"
#include "bitmap.h"
#include <stdbool.h>
#include <stdint.h>

struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

spinlock_t      spin_lock = SPINLOCK_INIT;
static uint8_t* bitmap = 0;
static uint64_t bitmap_top_index = 0;
static uint64_t bitmap_prev_index = 0;
// static uint64_t  used_pages = 0;
// static uint64_t* base_addr = 0;
// static uint64_t  total_pages = 0;
// static uint64_t  usable_pages = 0;
// static uint64_t  reserved_pages = 0;

void pmm_init() {
    struct limine_memmap_response* memmap = memmap_req.response;
    struct limine_hhdm_response*   hhdm = hhdm_req.response;
    struct limine_memmap_entry**   entries = memmap->entries;

    uint64_t highest_addr = 0;

    for (uint8_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];

        switch (entry->type) {
        case LIMINE_MEMMAP_USABLE:
            highest_addr = MAX(highest_addr, entry->base + entry->length);
            break;
        case LIMINE_MEMMAP_RESERVED:
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
        case LIMINE_MEMMAP_ACPI_NVS:
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            break;
        }
    }

    bitmap_top_index = highest_addr / PAGE_SIZE;
    uint64_t bitmap_size = ALIGN_UP(bitmap_top_index / 8, PAGE_SIZE);

    klog("PMM ::", "highest addr: %x", highest_addr);
    klog("PMM ::", "bitmap size: %d", bitmap_size);

    // Find a hole for the bitmap in the memory map.
    // Find a place for the bitmap to reside in.
    for (uint8_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length >= bitmap_size) {
            // This is where we determine where the free space for allocation
            // resides
            bitmap = (uint8_t*) (entry->base +
                                 HHDM_OFFSET); // offsetting the bitmap to the
                                               // higher half of the memory
            // Initialise entire bitmap to 1 (non-free)
            memset(bitmap, 0xff, bitmap_size);

            entry->length -=
                bitmap_size; // we occupied space for the btimap itself.
            entry->base += bitmap_size; // the start address of usable memory
                                        // that will be allocated.
            break;
        }
    }

    if (bitmap == 0) {
        panic("pmm_init: no space for bitmap");
    }

    // Populate free bitmap entries according to the memory map.
    for (uint8_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        // The entry is now of Usable type

        for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
            /* this is where all the magic happens */
            bitmap_clear(bitmap, (entry->base + j) / PAGE_SIZE);
        }
    }

    klog("PMM ::", "finished pmm initialization");
}

void* physical_alloc(uint64_t n_pages, uint64_t limit) {
    uint64_t consecutive_pages = 0;

    // not sure about pre_index
    while (bitmap_prev_index < limit) {
        if (bitmap_get(bitmap, bitmap_prev_index)) {
            consecutive_pages = 0;
            bitmap_prev_index++;
        } else {
            consecutive_pages++;
            bitmap_prev_index++;
            if (consecutive_pages == n_pages) {
                uint64_t page_index = bitmap_prev_index - n_pages;
                for (uint64_t i = page_index; i < page_index + n_pages; i++) {
                    bitmap_set(bitmap, i);
                }
                // this is going to be a physical address as the 'usable memory
                // entry' base address gives us physical address.
                return (void*) (page_index * PAGE_SIZE);
            }
        }
    }

    return NULL;
}

void* pmm_alloc_nozero(uint64_t n_pages) {
    spinlock_acquire(&spin_lock);

    uint64_t prev_index = bitmap_prev_index;
    void*    allocated = physical_alloc(n_pages, bitmap_top_index);

    if (allocated == NULL) {
        // wrap to the beginning of the bitmap because no more space left
        bitmap_prev_index = 0;
        allocated = physical_alloc(n_pages, prev_index);
        if (allocated == NULL) {
            panic("PMM_ALLOC: failed to allocate memory");
        }
    }

    spinlock_release(&spin_lock);

    return allocated;
}

void* pmm_alloc(uint64_t n_pages) {

    void* allocated = pmm_alloc_nozero(n_pages);
    if (allocated != NULL) {
        // memset(allocated, 0, n_pages * PAGE_SIZE);
        memset(allocated + HHDM_OFFSET, 0, n_pages * PAGE_SIZE);
    }

    return allocated;
}

void pmm_free(void* addr, uint64_t n_pages) {
    if (n_pages == 0) {
        return;
    }

    spinlock_acquire(&spin_lock);

    uint64_t bitmap_index = (uint64_t) addr / PAGE_SIZE;
    for (uint64_t i = bitmap_index; i < bitmap_index + n_pages; i++) {
        if (!bitmap_get(bitmap, i)) {
            panic("pmm_free: page already free");
        }
        bitmap_clear(bitmap, i);
    }

    spinlock_release(&spin_lock);
}

bool pmm_check_alloc(void* addr) {
    uint64_t bitmap_index = (uint64_t) addr / PAGE_SIZE;
    return bitmap_get(bitmap, bitmap_index);
}
