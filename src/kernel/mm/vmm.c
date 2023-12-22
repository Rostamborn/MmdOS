#include "../lib/util.h"
#include "../limine.h"
#include "pmm.h"
#include "vmm.h"
#include <stdbool.h>
#include <stdint.h>

static volatile struct limine_kernel_address_request kaddr_req = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

static uint64_t* lvl_get_next(uint64_t* top_lvl, uint64_t offset, bool alloc) {
    if (top_lvl[offset] & PTE_PRESENT) {
        return (uint64_t*)(PTE_GET_ADDR(top_lvl[offset]));
    }

    if (!alloc) {
        return NULL;
    }

    // Allocate a new PTE if it doesn't exist
    void* next_lvl = pmm_alloc(1);
    if (next_lvl == NULL) {
        return NULL;
    }

    top_lvl[offset] = (uint64_t)next_lvl | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    return next_lvl + HHDM_OFFSET; // HHDM_OFFSET is the higher half offset
}

static void lvl_destroy(PageMap* pagemap, uint16_t start, uint16_t end, uint16_t lvl) {
    // Base of Recursion
    if (lvl == 0) {
        return;
    }

    for (uint16_t i = start; i < end; i++) {
        uint64_t* next_lvl = lvl_get_next(pagemap->top_lvl, i, false);
        if (next_lvl == NULL) {
            continue;
        }

        lvl_destroy(pagemap, 0, 512, lvl - 1);
    }
    pmm_free((void*)pagemap - HHDM_OFFSET, 1);
}

void vmm_destroy_pagemap(PageMap* pagemap) {

    // maybe unmap first

    spinlock_acquire(&pagemap->lock);
    lvl_destroy(pagemap, 0, 512, 3);
    spinlock_release(&pagemap->lock);
}

void vmm_switch_pml(PageMap* pagemap) {
    // not sure about the HHDM_OFFSET
    __asm__ volatile("mov %0, %%cr3" ::"r"((void*)pagemap->top_lvl - HHDM_OFFSET): "memory");
}

bool vmm_map_page(PageMap* pagemap, uintptr_t virtual, uintptr_t physical, uint64_t flags) {
    spinlock_acquire(&pagemap->lock);

    bool ok = false;

    uint64_t pml_entries[4] = {
        (virtual & (NINE_BITS << 39)) >> 39,
        (virtual & (NINE_BITS << 30)) >> 30,
        (virtual & (NINE_BITS << 21)) >> 21,
        (virtual & (NINE_BITS << 12)) >> 12,
    };

    uint64_t* pml1 = NULL;

    for (uint16_t i = 0; i < 4; i++) {
        uint64_t* next_lvl = lvl_get_next(pagemap->top_lvl, pml_entries[i], true);
        if (next_lvl == NULL) {
            goto cleanup;
        }

        if (i == 3) {
            pml1 = next_lvl;
            if (next_lvl[pml_entries[i]] & PTE_PRESENT) {
                goto cleanup;
            }
        }
    }

    ok = true;
    pml1[pml_entries[3]] = physical | flags;

cleanup:
    // TODO: do some TLB stuff

    spinlock_release(&pagemap->lock);
    return ok;
}

bool vmm_unmap_page(PageMap* pagemap, uintptr_t virtual, bool locked) {
    if (!locked) {
        spinlock_acquire(&pagemap->lock);
    }

    bool ok = false;

    uint64_t pml_entries[4] = {
        (virtual & (NINE_BITS << 39)) >> 39,
        (virtual & (NINE_BITS << 30)) >> 30,
        (virtual & (NINE_BITS << 21)) >> 21,
        (virtual & (NINE_BITS << 12)) >> 12,
    };

    uint64_t* pml1 = NULL;

    for (uint16_t i = 0; i < 4; i++) {
        uint64_t* next_lvl = lvl_get_next(pagemap->top_lvl, pml_entries[i], false);
        if (next_lvl == NULL) {
            goto cleanup;
        }

        if (i == 3) {
            pml1 = next_lvl;
            if (!(next_lvl[pml_entries[i]] & PTE_PRESENT)) {
                goto cleanup;
            }
        }
    }

    ok = true;
    pml1[pml_entries[3]] = 0;

cleanup:
    // TODO: do some TLB stuff here

    if (!locked) {
        spinlock_release(&pagemap->lock);
    }
    return ok;
}

uint64_t* vmm_virtual2pte(PageMap* pagemap, uintptr_t virtual, bool alloc) {
    uint64_t pml_entries[4] = {
        (virtual & (NINE_BITS << 39)) >> 39,
        (virtual & (NINE_BITS << 30)) >> 30,
        (virtual & (NINE_BITS << 21)) >> 21,
        (virtual & (NINE_BITS << 12)) >> 12,
    };

    for (uint16_t i = 0; i < 4; i++) {
        uint64_t* next_lvl = lvl_get_next(pagemap->top_lvl, pml_entries[i], true);

        if (i == 3) {
            return &next_lvl[pml_entries[i]];
        }
    }

    return NULL;
}

uint64_t vmm_virtual2physical(PageMap* pagemap, uintptr_t virtual, bool alloc) {
    uint64_t* pte = vmm_virtual2pte(pagemap, virtual, alloc);
    if (pte == NULL) {
        // invalid physical address
        return 0;
    }

    return PTE_GET_ADDR(*pte);
}

void vmm_init() {

}
