#include "vmm.h"
#include "../cpu/cpu.h"
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

vmm_t* vmm_kernel = NULL;

extern char _text_start_addr[], _text_end_addr[];
extern char _rodata_start_addr[], _rodata_end_addr[];
extern char _data_start_addr[], _data_end_addr[];

extern char _kernel_start[];
extern char _kernel_end[];

static uint64_t* walk_pte(uint64_t* pte, uint64_t offset, bool alloc) {
    if (pte[offset] & PTE_PRESENT) {
        return (uint64_t*) (PTE_GET_ADDR(pte[offset]) + get_hhdm());
    }

    if (!alloc) {
        // we didn't find a present PTE and we don't want to allocate
        return NULL;
    }

    // Allocate a new PTE if it doesn't exist
    void* next_lvl = pmm_alloc(1); // the returned address is page aligned

    // not sure bout the PTE_USER flag
    pte[offset] = (uint64_t) next_lvl | PTE_PRESENT | PTE_WRITABLE;
    return (uint64_t*) (next_lvl + get_hhdm());
}

// TODO: this is the worst code I've written
vmm_t* vmm_new() {
    vmm_t* new_vmm = pmm_alloc(1);
    new_vmm = (vmm_t*) ((uintptr_t) new_vmm + get_hhdm());
    new_vmm->arena = NULL;
    new_vmm->lock = (spinlock_t) SPINLOCK_INIT;
    new_vmm->pml = pmm_alloc(1);
    // new_vmm->pml = (uint64_t*) ((uintptr_t) new_vmm->pml + get_hhdm());
    new_vmm->pml = (uint64_t*) ((uintptr_t) new_vmm->pml + get_hhdm());

    vmm_map_page(vmm_kernel, (uintptr_t) ((void*) new_vmm + get_hhdm()),
                 (uintptr_t) new_vmm, PTE_PRESENT | PTE_WRITABLE);

    vmm_map_page(vmm_kernel, (uintptr_t) ((void*) new_vmm->pml + get_hhdm()),
                 (uintptr_t) new_vmm->pml, PTE_PRESENT | PTE_WRITABLE);

    // TODO: i must be 256 to map only the higher half
    for (uint64_t i = 0; i < 512; i++) {
        new_vmm->pml[i] = vmm_kernel->pml[i];
    }

    return new_vmm;
}

static void destroy_lvl(uint64_t* lvl, uint64_t lvl_offset) {
    // basis of recursion
    if (lvl == 0) {

        return;
    }
    // NOTE: not sure, maybe a bug
    for (uint64_t i = 0; i < 512; i++) {
        if (lvl[i] & PTE_PRESENT) {
            uint64_t* next_lvl = (uint64_t*) (PTE_GET_ADDR(lvl[i]));
            destroy_lvl(next_lvl, lvl_offset - 1);
            pmm_free((void*) next_lvl - get_hhdm(), 1);
        }
    }
    pmm_free((void*) lvl - get_hhdm(), 1);
}

void vmm_destroy(vmm_t* vmm) {

    destroy_lvl(vmm->pml, 4);
    pmm_free((void*) vmm->arena - get_hhdm(), 1);
    pmm_free((void*) vmm - get_hhdm(), 1);
}

void vmm_switch_pml(vmm_t* vmm) {
    asm volatile(
        "mov %0, %%cr3" ::"r"((uint64_t) ((void*) vmm->pml - get_hhdm()))
        : "memory");
}

bool vmm_map_page(vmm_t* vmm, uintptr_t virt, uintptr_t physical,
                  uint64_t flags) {
    spinlock_acquire(&vmm->lock);

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (NINE_BITS << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (NINE_BITS << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (NINE_BITS << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (NINE_BITS << 12)) >> 12;

    uint64_t* lvl4 = vmm->pml;
    uint64_t* lvl3 = walk_pte(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = walk_pte(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = walk_pte(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        goto cleanup;
    }
    if (lvl1[lvl1_offset] & PTE_PRESENT) {
        // NOTE: maybe we should not set ok = true
        ok = true;
        goto cleanup;
    }

    ok = true;
    lvl1[lvl1_offset] = physical | flags;

cleanup:
    // TODO: do some TLB stuff

    // if (ok) {
    //     klog("VMM ::", "vmm_map_page: %p -> %p", virt, physical);
    // }
    spinlock_release(&vmm->lock);
    return ok;
}

bool vmm_unmap_page(vmm_t* vmm, uintptr_t virt, bool lock) {
    if (lock) {
        spinlock_acquire(&vmm->lock);
    }

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = vmm->pml;
    uint64_t* lvl3 = walk_pte(lvl4, lvl4_offset, false);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = walk_pte(lvl3, lvl3_offset, false);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = walk_pte(lvl2, lvl2_offset, false);
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

    if (lock) {
        spinlock_release(&vmm->lock);
    }
    return ok;
}

bool vmm_change_flag(vmm_t* vmm, uintptr_t virt, uint64_t flags, bool lock) {
    if (lock) {
        spinlock_acquire(&vmm->lock);
    }

    bool     ok = false;
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = vmm->pml;
    uint64_t* lvl3 = walk_pte(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl2 = walk_pte(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        goto cleanup;
    }
    uint64_t* lvl1 = walk_pte(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        goto cleanup;
    }
    if (!(lvl1[lvl1_offset] & PTE_PRESENT)) {
        goto cleanup;
    }

    ok = true;
    lvl1[lvl1_offset] = PTE_GET_ADDR(lvl1[lvl1_offset]) | flags;

cleanup:
    if (lock) {
        spinlock_release(&vmm->lock);
    }
    return ok;
}

uint64_t* vmm_virt2pte(vmm_t* vmm, uintptr_t virt, bool alloc) {
    uint64_t lvl4_offset = (virt & (0x1ffull << 39)) >> 39;
    uint64_t lvl3_offset = (virt & (0x1ffull << 30)) >> 30;
    uint64_t lvl2_offset = (virt & (0x1ffull << 21)) >> 21;
    uint64_t lvl1_offset = (virt & (0x1ffull << 12)) >> 12;

    uint64_t* lvl4 = vmm->pml;
    uint64_t* lvl3 = walk_pte(lvl4, lvl4_offset, true);
    if (lvl3 == NULL) {
        return NULL;
    }
    uint64_t* lvl2 = walk_pte(lvl3, lvl3_offset, true);
    if (lvl2 == NULL) {
        return NULL;
    }
    uint64_t* lvl1 = walk_pte(lvl2, lvl2_offset, true);
    if (lvl1 == NULL) {
        return NULL;
    }

    return &lvl1[lvl1_offset];
}

uint64_t vmm_virt2phys(vmm_t* vmm, uintptr_t virt, bool alloc) {
    uint64_t* pte = vmm_virt2pte(vmm, virt, alloc);
    if (pte == NULL) {
        // invalid physical address
        klog("VMM ::", "invalid physical address");
        return 0;
    }

    return PTE_GET_ADDR(*pte);
}

void vmm_init() {
    vmm_kernel = pmm_alloc(1);
    // this will be mapped by the end of vmm_init
    vmm_kernel = (vmm_t*) ((uintptr_t) vmm_kernel + get_hhdm());
    vmm_kernel->arena = NULL;
    vmm_kernel->lock = (spinlock_t) SPINLOCK_INIT;
    vmm_kernel->pml = pmm_alloc(1);
    vmm_kernel->pml = (uint64_t*) ((uintptr_t) vmm_kernel->pml + get_hhdm());

    // allocate the kernel page table(higher half)
    for (uint64_t i = 256; i < 512; i++) {
        if (walk_pte(vmm_kernel->pml, i, true) == NULL) {
            panic("Failed to allocate kernel page table");
        }
    }

    uintptr_t text_start = ALIGN_DOWN((uintptr_t) _text_start_addr, PAGE_SIZE),
              rodata_start =
                  ALIGN_DOWN((uintptr_t) _rodata_start_addr, PAGE_SIZE),
              data_start = ALIGN_DOWN((uintptr_t) _data_start_addr, PAGE_SIZE),
              text_end = ALIGN_UP((uintptr_t) _text_end_addr, PAGE_SIZE),
              rodata_end = ALIGN_UP((uintptr_t) _rodata_end_addr, PAGE_SIZE),
              data_end = ALIGN_UP((uintptr_t) _data_end_addr, PAGE_SIZE);

    struct limine_kernel_address_response* kaddr = kaddr_req.response;

    for (uintptr_t i = text_start; i < text_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        bool      res = vmm_map_page(vmm_kernel, i, physical, PTE_PRESENT);
        if (!res) {
            panic("Failed to map kernel text");
        }
    }

    for (uintptr_t i = rodata_start; i < rodata_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        bool      res =
            vmm_map_page(vmm_kernel, i, physical, PTE_PRESENT | PTE_NO_EXECUTE);
        if (!res) {
            panic("Failed to map kernel rodata");
        }
    }

    for (uintptr_t i = data_start; i < data_end; i += PAGE_SIZE) {
        uintptr_t physical = i - kaddr->virtual_base + kaddr->physical_base;
        if (!vmm_map_page(vmm_kernel, i, physical,
                          PTE_PRESENT | PTE_WRITABLE | PTE_NO_EXECUTE)) {
            panic("Failed to map kernel data");
        }
    }

    // TODO: do something so this lower half doesn't have an impact anymore
    for (uintptr_t i = 0x1000; i < 0x100000000; i += PAGE_SIZE) {
        bool res1 = vmm_map_page(vmm_kernel, i, i, PTE_PRESENT | PTE_WRITABLE);
        if (!res1) {
            panic("Failed to identity map the start of physical memory");
        }

        bool res2 = vmm_map_page(vmm_kernel, i + get_hhdm(), i,
                                 PTE_PRESENT | PTE_WRITABLE | PTE_NO_EXECUTE);
        if (!res2) {
            panic("Failed to map 0x1000 + get_hhdm() to 0x1000");
        }
    }

    // struct limine_memmap_response* memmap = memmap_req.response;
    // for (uint64_t i = 0; i < memmap->entry_count; i++) {
    //     struct limine_memmap_entry* entry = memmap->entries[i];
    //
    //     // identity map the higher half
    //     uintptr_t base = ALIGN_DOWN(entry->base, PAGE_SIZE);
    //     uintptr_t top = ALIGN_UP(entry->base + entry->length, PAGE_SIZE);
    //
    //     switch (entry->type) {
    //     case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
    //         for (uintptr_t i = base; i < top; i += PAGE_SIZE) {
    //             if (!vmm_map_page(vmm_kernel, i + get_hhdm(), i,
    //                               PTE_PRESENT | PTE_WRITABLE)) {
    //                 panic("bootloader reclaimable not mapped");
    //             }
    //             if (!vmm_map_page(vmm_kernel, i, i,
    //                               PTE_PRESENT | PTE_WRITABLE)) {
    //                 panic("bootloader reclaimable not mapped");
    //             }
    //         }
    //     case LIMINE_MEMMAP_FRAMEBUFFER:
    //         for (uintptr_t i = base; i < top; i += PAGE_SIZE) {
    //             if (!vmm_map_page(vmm_kernel, i + get_hhdm(), i,
    //                               PTE_PRESENT | PTE_WRITABLE)) {
    //                 panic("framebuffer not mapped");
    //             }
    //             if (!vmm_map_page(vmm_kernel, i, i,
    //                               PTE_PRESENT | PTE_WRITABLE)) {
    //                 panic("framebuffer not mapped");
    //             }
    //         }
    //     }
    //     if (top <= 0x100000000) {
    //         continue;
    //     }
    //
    //     for (uintptr_t j = base; j < top; j += PAGE_SIZE) {
    //         if (j < 0x100000000) {
    //             continue;
    //         }
    //
    //         if (!vmm_map_page(vmm_kernel, j, j, PTE_PRESENT | PTE_WRITABLE)) {
    //             panic("Failed to identity map physical memory");
    //         }
    //
    //         if (!vmm_map_page(vmm_kernel, j + get_hhdm(), j,
    //                           PTE_PRESENT | PTE_WRITABLE)) {
    //             panic("Failed to map physical memory");
    //         }
    //     }
    // }

    vmm_switch_pml(vmm_kernel);

    klog("VMM ::", "vmm init finished");
}
