#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H

#include "../lib/spinlock.h"
#include "../lib/util.h"
#include <stdint.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define NINE_BITS 0x1ffull
// Page Table Entry
#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITABLE (1ull << 1ull)
#define PTE_USER (1ull << 2ull)
#define PTE_NO_EXECUTE (1ull << 63ull)
#define PTE_ADDR 0x000ffffffffff000ull

// neat trick
#define PTE_GET_ADDR(pte) ((pte) &PTE_ADDR)
#define PTE_GET_FLAGS(pte) ((pte) & ~PTE_ADDR)

typedef struct {
    spinlock_t lock;
    uint64_t* pml;
} vmm_t;

typedef struct {
    uintptr_t address;
    char*     name;
} symbol;

void vmm_init();

void vmm_destroy_pagemap(vmm_t* pagemap);

void vmm_switch_pml(vmm_t* pagemap);

bool vmm_map_page(vmm_t* pagemap, uintptr_t virt, uintptr_t physical,
                  uint64_t flags);

bool vmm_unmap_page(vmm_t* pagemap, uintptr_t virt, bool locked);

uint64_t* vmm_virt2pte(vmm_t* pagemap, uintptr_t virt, bool alloc);

uint64_t vmm_virt2physical(vmm_t* pagemap, uintptr_t virt, bool alloc);

#endif
