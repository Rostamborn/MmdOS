#include "src/kernel/mm/pmm.h"
#include "src/kernel/cpu/cpu.h"
#include "src/kernel/lib/logger.h"
#include "src/kernel/lib/panic.h"
#include "src/kernel/lib/spinlock.h"
#include "src/kernel/lib/util.h"
#include "src/kernel/limine.h"
#include "src/kernel/mm/bitmap.h"
#include <stdint.h>

struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

spinlock_t       spin_lock = SPINLOCK_INIT;
static uint8_t*  bitmap = 0;
static uint64_t* base_addr = 0;
static uint64_t  page_index_limit = 0;
static uint64_t  prev_page_index = 0;
static uint64_t  total_pages = 0;
static uint64_t  usable_pages = 0;
static uint64_t  used_pages = 0;
static uint64_t  reserved_pages = 0;

void pmm_init() {
    struct limine_memmap_response* memmap = memmap_req.response;
    // struct limine_hhdm_response*   hhdm = hhdm_req.response;
    struct limine_memmap_entry** entries = memmap->entries;

    uint64_t highest_addr = 0;

    for (uint8_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];

        /* For some reason, entry->Type seems to be NULL */

        klog("PMM :: ", "addr: %x, size: %d", entry->base, entry->length,
             entry->type);
        // if(entry->type != 1) continue;
        switch (entry->type) {
        case LIMINE_MEMMAP_USABLE:
            usable_pages += DIV_ROUNDUP(entry->length, PAGE_SIZE);
            highest_addr = MAX(highest_addr, entry->base + entry->length);
            break;
        case LIMINE_MEMMAP_RESERVED:
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
        case LIMINE_MEMMAP_ACPI_NVS:
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            reserved_pages += DIV_ROUNDUP(entry->length, PAGE_SIZE);
            break;
        }
    }

    page_index_limit = highest_addr / PAGE_SIZE;
    uint64_t bitmap_size = ALIGN_UP(page_index_limit / 8, PAGE_SIZE);
    // page_align_up(page_index_limit / 8); // becaue we use 1 bit per page
    // uint64_t bitmap_size = div_round_up(page_index_limit , 8); // becaue we
    // use 1 bit per page

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
            base_addr = (uint64_t*) entry->base;

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

    klog("PMM ::", "usable memory: %d Mib",
         (usable_pages * PAGE_SIZE) / 1024 / 1024);

    klog("PMM ::", "Base Address: %x", base_addr);
}

void* physical_alloc(uint64_t n_pages, uint64_t limit) {
    uint64_t consecutive_pages = 0;

    // not sure about pre_index
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
                return (void*) (page_index *
                                PAGE_SIZE); // the base addr of allocated pages
            }
        }
    }

    return NULL;
}

void* pmm_alloc(uint64_t n_pages) {
    spinlock_acquire(&spin_lock);

    uint64_t prev_index = prev_page_index;
    void*    allocated = physical_alloc(n_pages, page_index_limit);

    if (allocated ==
        NULL) { // for when we hit the page_index_limit or can't find enougth
                // n_pages in the current range. so we wrap to the beginning of
                // the bitmap till the prev_page_index
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

    // klog(0, "pmm: allocated %d pages", n_pages);
    // klog(0, "pmm: used memory: %d MiB", (used_pages * PAGE_SIZE) / 1024 /
    // 1024); klog(0,"pmm: remaining memory: %d MiB\n", (usable_pages *
    // PAGE_SIZE) / 1024 / 1024);

    return allocated;
}

void* pmm_alloc_nozero(uint64_t n_pages) {
    spinlock_acquire(&spin_lock);

    uint64_t prev_index = prev_page_index;
    void*    allocated = physical_alloc(n_pages, page_index_limit);

    if (allocated ==
        NULL) { // for when we hit the page_index_limit or can't find enougth
                // n_pages in the current range. so we wrap to the beginning of
                // the bitmap till the prev_page_index
        prev_page_index = 0;
        allocated = physical_alloc(n_pages, prev_index);
    }

    if (allocated != NULL) {
        used_pages += n_pages;
    } else {
        panic("pmm_alloc_nozero: out of memory");
    }

    spinlock_release(&spin_lock);

    // klog(0, "pmm: allocated %d pages", n_pages);
    // klog(0, "pmm: used memory: %d MiB", (used_pages * PAGE_SIZE) / 1024 /
    // 1024); klog(0,"pmm: remaining memory: %d MiB\n", (usable_pages *
    // PAGE_SIZE) / 1024 / 1024);

    return allocated;
}

void pmm_free(void* addr, uint64_t n_pages) {
    if (n_pages == 0) {
        return;
    }

    spinlock_acquire(&spin_lock);

    uint64_t page_index = (uint64_t) addr / PAGE_SIZE;
    for (uint64_t i = page_index; i < page_index + n_pages; i++) {
        if (!bitmap_get(bitmap, i)) {
            panic("pmm_free: page already free");
        }
        bitmap_clear(bitmap, i);
    }
    used_pages -= n_pages;

    spinlock_release(&spin_lock);
}
