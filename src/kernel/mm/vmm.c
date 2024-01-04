#include "vmm.h"
#include "../lib/alloc.h"
#include "../lib/logger.h"
#include "../lib/panic.h"
#include "../lib/util.h"
#include "../limine.h"
#include "pmm.h"
#include <stdbool.h>
#include <stdint.h>

static volatile struct limine_kernel_address_request kaddr_req = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

PageMap* vmm_kernel_pagemap = NULL;

extern uint64_t _text_start_addr, _text_end_addr;
extern uint64_t _rodata_start_addr, _rodata_end_addr;
extern uint64_t _data_start_addr, _data_end_addr;

static uint64_t* lvl_get_next(uint64_t* top_lvl, uint64_t offset, bool alloc) {
    if ((top_lvl[offset] & PTE_PRESENT) != 0) {
        return (uint64_t*) (PTE_GET_ADDR(top_lvl[offset]) + HHDM_OFFSET);
    }

    if (!alloc) {
        return NULL;
    }

    // Allocate a new PTE if it doesn't exist
    void* next_lvl = pmm_alloc(1); // the returned address is page aligned
    if (next_lvl == NULL) {
        return NULL;
    }

    // not sure bout the PTE_USER flag
    top_lvl[offset] = (uint64_t)next_lvl | PTE_PRESENT | PTE_WRITABLE /* | PTE_USER */;
    return (uint64_t*)(next_lvl + HHDM_OFFSET); // HHDM_OFFSET is the higher half offset
}

static void lvl_destroy(PageMap* pagemap, uint16_t start, uint16_t end,
                        uint16_t lvl) {
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
    pmm_free((void*) pagemap - HHDM_OFFSET, 1);
}

void vmm_destroy_pagemap(PageMap* pagemap) {

    // maybe unmap first

    spinlock_acquire(&pagemap->lock);
    lvl_destroy(pagemap, 0, 512, 3);
    spinlock_release(&pagemap->lock);
}

// void vmm_switch_pml(PageMap* pagemap) {
//     // not sure about the HHDM_OFFSET
//     __asm__ volatile(
//         "mov %0, %%cr3" ::"r"((uintptr_t) &pagemap->top_lvl - HHDM_OFFSET)
//         : "memory");
// }
void vmm_switch_pml(PageMap* pagemap) {
    asm volatile (
        "mov %0, %%cr3"
        :
        : "r" ((uint64_t)((void*)pagemap->top_lvl - HHDM_OFFSET))
        : "memory"
    );
}

bool vmm_map_page(PageMap* pagemap, uintptr_t virt, uintptr_t physical,
                  uint64_t flags) {
    spinlock_acquire(&pagemap->lock);

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = pagemap->top_lvl;
    uint64_t* lvl3 = lvl_get_next(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = lvl_get_next(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = lvl_get_next(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        goto cleanup;
    }

    if ((lvl1[lvl1_offset] & PTE_PRESENT) != 0) {
        goto cleanup;
    }

    ok = true;
    lvl1[lvl1_offset] = physical | flags;

cleanup:
    // TODO: do some TLB stuff

    if (ok) {
        klog("VMM ::", "vmm_map_page: %p -> %p", virt, physical);
    }
    spinlock_release(&pagemap->lock);
    return ok;
}

bool vmm_unmap_page(PageMap* pagemap, uintptr_t virt, bool locked) {
    if (!locked) {
        spinlock_acquire(&pagemap->lock);
    }

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = pagemap->top_lvl;
    uint64_t* lvl3 = lvl_get_next(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = lvl_get_next(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = lvl_get_next(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        goto cleanup;
    }

    if ((lvl1[lvl1_offset] & PTE_PRESENT) != 0) {
        goto cleanup;
    }

    ok = true;
    lvl1[lvl1_offset] = 0;

cleanup:
    // TODO: do some TLB stuff here

    if (!locked) {
        spinlock_release(&pagemap->lock);
    }
    return ok;
}

uint64_t* vmm_virtual2pte(PageMap* pagemap, uintptr_t virt, bool alloc) {
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = pagemap->top_lvl;
    uint64_t* lvl3 = lvl_get_next(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        return NULL;
    }
    uint64_t* lvl2 = lvl_get_next(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        return NULL;
    }
    uint64_t* lvl1 = lvl_get_next(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        return NULL;
    }

    return &lvl1[lvl1_offset];
}

uint64_t vmm_virtual2physical(PageMap* pagemap, uintptr_t virt, bool alloc) {
    uint64_t* pte = vmm_virtual2pte(pagemap, virt, alloc);
    if (pte == NULL) {
        // invalid physical address
        klog("VMM ::", "invalid physical address");
        return 0;
    }

    return PTE_GET_ADDR(*pte);
}

void vmm_init() {
    vmm_kernel_pagemap = KALLOC(PageMap);
    vmm_kernel_pagemap->lock = (spinlock_t) SPINLOCK_INIT;
    vmm_kernel_pagemap->top_lvl = pmm_alloc(1);

    if (vmm_kernel_pagemap->top_lvl == NULL) {
        panic("Failed to allocate top level page table");
    }
    vmm_kernel_pagemap->top_lvl = (void*)vmm_kernel_pagemap->top_lvl + HHDM_OFFSET;

    // allocate the kernel page table(higher half)
    for (uint64_t i = 256; i < 512; i++) {
        if (lvl_get_next(vmm_kernel_pagemap->top_lvl, i, true) == NULL) {
            panic("Failed to allocate kernel page table");
        }
    }
    klog("VMM ::", "Allocated kernel PML entries");

    struct limine_kernel_address_response* kaddr = kaddr_req.response;

    // klog(0, "text_start_addr: %p", &_text_start_addr);
    // klog(0, "rodata_start_addr: %p", &_rodata_start_addr);
    // klog(0, "data_start_addr: %p", &_data_start_addr);

    uintptr_t text_start =
                  ALIGN_DOWN((uintptr_t) (void*) &_text_start_addr, PAGE_SIZE),
              rodata_start = ALIGN_DOWN((uintptr_t) (void*) &_rodata_start_addr,
                                        PAGE_SIZE),
              data_start =
                  ALIGN_DOWN((uintptr_t) (void*) &_data_start_addr, PAGE_SIZE),
              text_end =
                  ALIGN_UP((uintptr_t) (void*) &_text_end_addr, PAGE_SIZE),
              rodata_end =
                  ALIGN_UP((uintptr_t) (void*) &_rodata_end_addr, PAGE_SIZE),
              data_end =
                  ALIGN_UP((uintptr_t) (void*) &_data_end_addr, PAGE_SIZE);

    klog("VMM ::", "text_start: %x", text_start);
    klog("VMM ::", "text_end: %x", text_end);
    klog("VMM ::", "rodata_start: %x", rodata_start);
    klog("VMM ::", "data_start: %x", data_start);
    //
    // I don't know why physical addresses are like this
    for (uintptr_t i = text_start; i < text_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        bool res = vmm_map_page(vmm_kernel_pagemap, i, physical, PTE_PRESENT);
        if (!res) {
            panic("Failed to map kernel text");
        }
        // klog(NULL, "text: virtual %x -> physical %x",
        // vmm_virtual2physical(vmm_kernel_pagemap, i, false), physical);
    }
    klog("VMM ::", "text mapped");

    for (uintptr_t i = rodata_start; i < rodata_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        bool      res = vmm_map_page(vmm_kernel_pagemap, i, physical,
                                     PTE_PRESENT | PTE_NO_EXECUTE);
        if (!res) {
            panic("Failed to map kernel rodata");
        }
    }
    klog("VMM ::", "rodata mapped");

    for (uintptr_t i = data_start; i < data_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        if (!vmm_map_page(vmm_kernel_pagemap, i, physical,
                          PTE_PRESENT | PTE_WRITABLE | PTE_NO_EXECUTE)) {
            panic("Failed to map kernel data");
        }
    }
    klog("VMM ::", "data mapped");

    // // *** Identity mapping all of physical memory ***
    // for (uintptr_t i = 0x1000; i < 0x100000000; i += PAGE_SIZE) {
    //     bool res1 = vmm_map_page(vmm_kernel_pagemap, i, i, PTE_PRESENT |
    //     PTE_WRITABLE); if (!res1) {
    //         panic("Failed to identity map the start of physical memory");
    //     }
    //
    //     bool res2 = vmm_map_page(vmm_kernel_pagemap, i + HHDM_OFFSET, i,
    //     PTE_PRESENT | PTE_WRITABLE | PTE_NO_EXECUTE); if (!res2) {
    //         panic("Failed to map 0x1000 + HHDM_OFFSET to 0x1000");
    //     }
    // }
    // klog("VMM ::", "rest of memory mapped");

    struct limine_memmap_response* memmap = memmap_req.response;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];

        // identity map the higher half
        uintptr_t base = ALIGN_DOWN(entry->base, PAGE_SIZE);
        uintptr_t top = ALIGN_UP(entry->base + entry->length, PAGE_SIZE);
        klog("VMM", "MAPPING %x - %x", base, top);
        // if (top <= 0x100000000) {
        //     continue;
        // }
        klog("VMM", "TYPE: %d", entry->type);
        if (entry->type != LIMINE_MEMMAP_USABLE || entry->type != LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            continue;
        }
        klog("VMM ::", "base and top set");

        for (uintptr_t j = base; j < top; j += PAGE_SIZE) {
            // if (j < 0x100000000) {
            //     continue;
            // }

            bool res1 = vmm_map_page(vmm_kernel_pagemap, j, j,
                                     PTE_PRESENT | PTE_WRITABLE);
            if (!res1) {
                panic("Failed to identity map physical memory");
            }

            klog("VMM ::", "res1 entry mapped");

            bool res2 =
                vmm_map_page(vmm_kernel_pagemap, j + HHDM_OFFSET, j,
                             PTE_PRESENT | PTE_WRITABLE | PTE_NO_EXECUTE);
            if (!res2) {
                panic("Failed to map physical memory");
            }
            klog("VMM ::", "res2 entry mapped");
        }
    }
    klog("VMM ::", "limine_memmap higher half mapped");


    // vmm_switch_pml(vmm_kernel_pagemap);
    klog("VMM ::", "switched pagemap");

    klog("VMM ::", "vmm init finished");
    // klog("VMM :: ", "text: virtual %x -> physical %x", text_start,
    // vmm_virtual2physical(vmm_kernel_pagemap, text_start, false));
}
