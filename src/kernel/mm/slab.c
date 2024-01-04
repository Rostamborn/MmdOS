#include "../lib/logger.h"
#include "../lib/spinlock.h"
#include "../lib/util.h"
#include "../lib/print.h"
#include "../limine.h"
#include "pmm.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct {
    spinlock_t spin_lock;
    void**     free_entry;
    uint64_t   entry_size;
} slab;

typedef struct {
    slab* slab;
} slab_header;

typedef struct {
    uint64_t n_pages;
    uint64_t size;
} allocation_info;

static slab slabs[10];

static inline slab* slab_search(uint64_t size) {
    for (uint64_t i = 0; i < SIZEOF_ARRAY(slabs); i++) {
        if (slabs[i].entry_size >= size) {
            return &slabs[i];
        }
    }
    return NULL;
}

static void create_slab(slab* slab, uint64_t entry_size) {
    slab->spin_lock = (spinlock_t) SPINLOCK_INIT;
    slab->free_entry = pmm_alloc_nozero(1) + HHDM_OFFSET;
    slab->entry_size = entry_size;

    uint64_t header_offset = ALIGN_UP(sizeof(slab_header), entry_size);
    uint64_t available_size = PAGE_SIZE - header_offset;

    slab_header* slab_ptr = (slab_header*) slab->free_entry;
    slab_ptr->slab = slab;
    slab->free_entry = (void**) ((void*) slab->free_entry + header_offset);

    void**   entries = (void**) slab->free_entry;
    uint64_t max_entries =
        (available_size / entry_size) - 1; // -1 because of slab_header
    uint64_t factor = entry_size / sizeof(void*);

    for (uint64_t i = 0; i < max_entries; i++) {
        entries[i * factor] = &entries[(i + 1) * factor];
    }
    entries[max_entries * factor] = NULL; // null terminate the list
}

static void* alloc_from_slab(slab* slab) {
    spinlock_acquire(&slab->spin_lock);

    if (slab->free_entry == NULL) {
        create_slab(slab, slab->entry_size);
    }

    void* entry = slab->free_entry;
    // not sure about this
    slab->free_entry = *slab->free_entry;
    memset(entry, 0, slab->entry_size);

    spinlock_release(&slab->spin_lock);

    return entry;
}

static void free_in_slab(slab* slab, void* addr) {
    spinlock_acquire(&slab->spin_lock);

    if (addr == NULL) {
        goto cleanup;
    }

    void** entry = (void**) addr;
    *entry = slab->free_entry;
    slab->free_entry = entry;

cleanup:
    spinlock_release(&slab->spin_lock);
}

void* slab_alloc(uint64_t size) {
    slab* slab = slab_search(size);
    if (slab != NULL) {
        return alloc_from_slab(slab);
    }

    // if we can't find a slab, then we allocate a page
    uint64_t n_pages = DIV_ROUNDUP(size, PAGE_SIZE);
    void*    res = pmm_alloc(n_pages + 1); // +1 because of the metadata
    if (res == NULL) {
        return NULL;
    }

    res += HHDM_OFFSET;

    // The metadata(alloc_info) resides in a page
    allocation_info* alloc_info = (allocation_info*) res;
    alloc_info->n_pages = n_pages;
    alloc_info->size = size;

    return res + PAGE_SIZE;
}

void slab_free(void* addr) {
    if (addr == NULL) {
        return;
    }

    // if the lower 12 bits are 0, then it's a slab, otherwise a slab_ptr
    if (((uintptr_t) addr & 0xfff) == 0) {
        allocation_info* alloc_info = (allocation_info*) (addr - PAGE_SIZE);
        pmm_free((void*) alloc_info - HHDM_OFFSET,
                 alloc_info->n_pages + 1); // +1 becuase of the metadata
        return;
    }

    slab_header* slab_ptr = (slab_header*) ((uintptr_t) addr & ~0xfff);
    free_in_slab(slab_ptr->slab, addr);
}

void* slab_realloc(void* addr, uint64_t new_size) {
    if (addr == NULL) {
        return slab_alloc(new_size);
    }

    // if the lower 12 bits are 0, then it's a slab, otherwise a slab_ptr
    if (((uintptr_t) addr & 0xfff) == 0) {
        allocation_info* alloc_info = (allocation_info*) (addr - PAGE_SIZE);
        if (DIV_ROUNDUP(alloc_info->size, PAGE_SIZE) ==
            DIV_ROUNDUP(new_size, PAGE_SIZE)) {
            alloc_info->size = new_size;
            return addr;
        }

        void* new_addr = slab_alloc(new_size);
        if (new_addr == NULL) {
            klog(0, "slab_realloc: out of memory");
            return NULL;
        }

        if (alloc_info->size > new_size) {
            memcpy(new_addr, addr, new_size);
        } else {
            memcpy(new_addr, addr, alloc_info->size);
        }

        slab_free(addr);
        return new_addr;
    }

    slab_header* slab_ptr = (slab_header*) ((uintptr_t) addr & ~0xfff);
    slab*        slab = slab_ptr->slab;

    if (new_size >= slab->entry_size) {
        void* new_addr = slab_alloc(new_size);
        if (new_addr == NULL) {
            klog(0, "slab_realloc: out of memory");
            return NULL;
        }
        memcpy(new_addr, addr, slab->entry_size);
        free_in_slab(slab, addr);
        return new_addr;
    }

    return addr;
}

void slab_init() {
    create_slab(&slabs[0], 8);
    create_slab(&slabs[1], 16);
    create_slab(&slabs[2], 24);
    create_slab(&slabs[3], 32);
    create_slab(&slabs[4], 48);
    create_slab(&slabs[5], 64);
    create_slab(&slabs[6], 128);
    create_slab(&slabs[7], 256);
    create_slab(&slabs[8], 512);
    create_slab(&slabs[9], 1024);
}
