#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H

#include "../lib/spinlock.h"
#include "../lib/util.h"
#include <stdint.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define MAXIMUM_VM_OBJECT 64
#define INITIAL_MAPPING_COUNT 0x100000

#define NINE_BITS 0x1ffull
// Page Table Entry
#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITABLE (1ull << 1ull) // everything is always readable
#define PTE_USER (1ull << 2ull) // if set, user can access
// #define PTE_WRITETHROUGH (1ull << 3ull) // these are related to PAT
// #define PTE_CACHE_DISABLED (1ull << 4ull)
// #define PTE_PAT (1ull << 7ull)
#define PTE_NO_EXECUTE (1ull << 63ull)
#define PTE_ADDR 0x000ffffffffff000ull

// neat trick
#define PTE_GET_ADDR(pte) ((pte) &PTE_ADDR)
#define PTE_GET_FLAGS(pte) ((pte) & ~PTE_ADDR)

#define VM_NONE 0
#define VM_WRITABLE (1ull << 0ull)
#define VM_EXEC (1ull << 1ull)
#define VM_USER (1ull << 2ull)

typedef struct vm_arena {
    uint64_t id;
    uintptr_t base;
    uint64_t size;
    uint64_t flags;
    uint64_t offset;
    uint64_t allocated;
    struct vm_arena* next;
} vm_arena;

typedef struct {
    spinlock_t lock;
    uint64_t arena_count;
    uint64_t*  lower_lvl;
    vm_arena* arenas;
} PageMap;

extern PageMap* vmm_kernel;

void vmm_init();

PageMap* vmm_get_pagemap();

void vmm_destroy_pagemap(PageMap* pagemap);

void vmm_switch_pml(PageMap* pagemap);

bool vmm_map(PageMap* pagemap, uintptr_t virt, uintptr_t physical,
                  uint64_t flags);

bool vmm_unmap(PageMap* pagemap, uintptr_t virt, bool locked);

// void* vmm_alloc(PageMap* pagemap, uint64_t size, uint64_t flags, void* arg);

// void vmm_free(PageMap* pagemap, void* addr);

bool vmm_set_page_flag(PageMap* pagemap, uintptr_t virt, uint64_t flag, bool locked);

uint64_t* vmm_virt2pte(PageMap* pagemap, uintptr_t virt, bool alloc);

uint64_t vmm_virt2phys(PageMap* pagemap, uintptr_t virt, bool alloc);

PageMap* vmm_new_pagemap();

uint64_t convert_flags(uint64_t flags);

#endif
