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

PageMap* vmm_kernel = NULL;

typedef char linker_addr[];
extern linker_addr _text_start_addr, _text_end_addr;
extern linker_addr _rodata_start_addr, _rodata_end_addr;
extern linker_addr _data_start_addr, _data_end_addr;

static uint64_t* pagemap_next(uint64_t* lower_lvl, uint64_t offset, bool alloc) {
    if (lower_lvl[offset] & PTE_PRESENT) {
        return (uint64_t*)(PTE_GET_ADDR(lower_lvl[offset]) + HHDM_OFFSET);
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
    lower_lvl[offset] = (uint64_t) next_lvl | PTE_PRESENT | PTE_WRITABLE /* | PTE_USER */;
    return (uint64_t*) (next_lvl + HHDM_OFFSET); // HHDM_OFFSET is the higher half offset
}

// static void lvl_destroy(PageMap* pagemap, uint16_t start, uint16_t end,
//                         uint16_t lvl) {
//     // Base of Recursion
//     if (lvl == 0) {
//         return;
//     }
//
//     for (uint16_t i = start; i < end; i++) {
//         uint64_t* next_lvl = lvl_get_next(pagemap->top_lvl, i, false);
//         if (next_lvl == NULL) {
//             continue;
//         }
//
//         lvl_destroy(pagemap, 0, 512, lvl - 1);
//     }
//     pmm_free((void*) pagemap - HHDM_OFFSET, 1);
// }
//
// void vmm_destroy_pagemap(PageMap* pagemap) {
//
//     // maybe unmap first
//
//     spinlock_acquire(&pagemap->lock);
//     lvl_destroy(pagemap, 0, 512, 3);
//     spinlock_release(&pagemap->lock);
// }
//
void vmm_switch_pml(PageMap* pagemap) {
    asm volatile("mov %0, %%cr3"
                 :
                 : "r"((uint64_t) ((void*) pagemap->lower_lvl - HHDM_OFFSET)) // getting phys addr by subracting HHDM_OFFSET
                 : "memory");
}

bool vmm_map(PageMap* pagemap, uintptr_t virt, uintptr_t physical,
                  uint64_t flags) {
    spinlock_acquire(&pagemap->lock);

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = pagemap->lower_lvl;
    uint64_t* lvl3 = pagemap_next(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = pagemap_next(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = pagemap_next(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        goto cleanup;
    }

    // if it is present, we don't do anything
    if (lvl1[lvl1_offset] & PTE_PRESENT) {
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

bool vmm_unmap(PageMap* pagemap, uintptr_t virt, bool locked) {
    if (!locked) {
        spinlock_acquire(&pagemap->lock);
    }

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = pagemap->lower_lvl;
    uint64_t* lvl3 = pagemap_next(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = pagemap_next(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = pagemap_next(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        goto cleanup;
    }

    if (!(lvl1[lvl1_offset] & PTE_PRESENT)) {
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

bool vmm_set_page_flag(PageMap* pagemap, uintptr_t virt, uint64_t flag, bool locked) {
    if (!locked) {
        spinlock_acquire(&pagemap->lock);
    }

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = pagemap->lower_lvl;
    uint64_t* lvl3 = pagemap_next(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = pagemap_next(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = pagemap_next(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        goto cleanup;
    }

    if (!(lvl1[lvl1_offset] & PTE_PRESENT)) {
        goto cleanup;
    }

    ok = true;
    lvl1[lvl1_offset] |= flag;

cleanup:
    if (!locked) {
        spinlock_release(&pagemap->lock);
    }
    return ok;
}

uint64_t* vmm_virt2pte(PageMap* pagemap, uintptr_t virt, bool alloc) {
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = pagemap->lower_lvl;
    uint64_t* lvl3 = pagemap_next(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        return NULL;
    }
    uint64_t* lvl2 = pagemap_next(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        return NULL;
    }
    uint64_t* lvl1 = pagemap_next(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        return NULL;
    }

    return &lvl1[lvl1_offset];
}

uint64_t vmm_virt2phys(PageMap* pagemap, uintptr_t virt, bool alloc) {
    uint64_t* pte = vmm_virt2pte(pagemap, virt, alloc);
    if (pte == NULL) {
        // invalid physical address
        klog("VMM ::", "invalid physical address");
        return 0;
    }

    return PTE_GET_ADDR(*pte);
}

PageMap* vmm_new_pagemap() {
    PageMap* pagemap = KALLOC(PageMap);
    if (pagemap == NULL) {
        return NULL;
    }

    pagemap->lock = (spinlock_t) SPINLOCK_INIT;
    pagemap->lower_lvl = pmm_alloc(1);
    if (pagemap->lower_lvl == NULL) {
        kfree(pagemap);
        return NULL;
    }
    pagemap->lower_lvl = (void*)pagemap->lower_lvl + HHDM_OFFSET;

    // shared kernel mappings for all processes
    for (uint64_t i = 256; i < 512; i++) {
        pagemap->lower_lvl[i] = vmm_kernel->lower_lvl[i];
    }

    return pagemap;
}

void vmm_init() {
    bool ok = false;
    vmm_kernel = KALLOC(PageMap);
    if (vmm_kernel == NULL) {
        panic("Failed to allocate kernel root pagemap");
    }

    vmm_kernel->lock = (spinlock_t) SPINLOCK_INIT;
    vmm_kernel->lower_lvl = pmm_alloc(1);
    if (vmm_kernel->lower_lvl == NULL) {
        kfree(vmm_kernel);
        panic("Failed to allocate kernel pml4");
    }
    vmm_kernel->lower_lvl = (void*)vmm_kernel->lower_lvl + HHDM_OFFSET;

    // allocate the kernel page table(higher half)
    for (uint64_t i = 256; i < 512; i++) {
        if (pagemap_next(vmm_kernel->lower_lvl, i, true) == NULL) {
            panic("Failed to allocate kernel page table");
        }
    }
    klog("VMM ::", "Allocated kernel PML entries");

    struct limine_kernel_address_response* kaddr = kaddr_req.response;
    klog("VMM ::", "_text_start_addr: %x", _text_start_addr);
    klog("VMM ::", "_text_end_addr: %x", _text_end_addr);
    klog("VMM ::", "_rodata_start_addr: %x", _rodata_start_addr);
    klog("VMM ::", "_rodata_end_addr: %x", _rodata_end_addr);
    klog("VMM ::", "_data_start_addr: %x", _data_start_addr);
    klog("VMM ::", "_data_end_addr: %x", _data_end_addr);

    uintptr_t text_start =
                  ALIGN_DOWN((uintptr_t)_text_start_addr, PAGE_SIZE),
              rodata_start = ALIGN_DOWN((uintptr_t)_rodata_start_addr,
                                        PAGE_SIZE),
              data_start =
                  ALIGN_DOWN((uintptr_t)_data_start_addr, PAGE_SIZE),
              text_end =
                  ALIGN_UP((uintptr_t)_text_end_addr, PAGE_SIZE),
              rodata_end =
                  ALIGN_UP((uintptr_t)_rodata_end_addr, PAGE_SIZE),
              data_end =
                  ALIGN_UP((uintptr_t)_data_end_addr, PAGE_SIZE);

    klog("VMM ::", "text_start: %x", text_start);
    klog("VMM ::", "text_end: %x", text_end);
    klog("VMM ::", "rodata_start: %x", rodata_start);
    klog("VMM ::", "data_start: %x", data_start);


    for (uintptr_t i = text_start; i < text_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        if (!vmm_map(vmm_kernel, i, physical, PTE_PRESENT)) {
            panic("Failed to map kernel text");
        }
    }
    klog("VMM ::", "text mapped");

    for (uintptr_t i = rodata_start; i < rodata_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        if (!vmm_map(vmm_kernel, i, physical, PTE_PRESENT | PTE_NO_EXECUTE)) {
            panic("Failed to map kernel rodata");
        }
    }
    klog("VMM ::", "rodata mapped");

    for (uintptr_t i = data_start; i < data_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        if (!vmm_map(vmm_kernel, i, physical, PTE_PRESENT | PTE_WRITABLE | PTE_NO_EXECUTE)) {
            panic("Failed to map kernel data");
        }
    }
    klog("VMM ::", "data mapped");

    for(uintptr_t i = 0x1000; i < 0x100000000; i += PAGE_SIZE) {
        if (!vmm_map(vmm_kernel, i, i, PTE_PRESENT | PTE_WRITABLE)) {
            panic("Failed to map kernel data");
        }
        if (!vmm_map(vmm_kernel, i + HHDM_OFFSET, i, PTE_PRESENT | PTE_WRITABLE)) {
            panic("Failed to map kernel data");
        }
    }

    struct limine_memmap_response* memmap = memmap_req.response;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        uintptr_t start = ALIGN_UP(entry->base, PAGE_SIZE),
                  end = ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE);

        for (uintptr_t i = start; i < end; i += PAGE_SIZE) {
            if (!vmm_map(vmm_kernel, i, i, PTE_PRESENT | PTE_WRITABLE)) {
                panic("Failed to map usable memory");
            }

            if (!vmm_map(vmm_kernel, i + HHDM_OFFSET, i, PTE_PRESENT | PTE_WRITABLE)) {
                panic("Failed to map usable memory");
            }
        }
    }

    vmm_switch_pml(vmm_kernel);
    klog("VMM ::", "Switched to kernel PML");

    klog("VMM ::", "Initialized VMM");
}
