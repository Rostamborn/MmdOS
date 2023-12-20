#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H

#include "../lib/spinlock.h"
#include "../lib/util.h"
#include <stdint.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

// Page Table Entry
#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITABLE (1ull << 1ull)
#define PTE_USER (1ull << 2ull)
#define PTE_NO_EXECUTE (1ull << 63ull)
#define PTE_ADDR_MASK 0x000ffffffffff000ull

// neat trick
#define PTE_GET_ADDR(pte) ((pte) &PTE_ADDR_MASK)
#define PTE_GET_FLAGS(pte) ((pte) & ~PTE_ADDR_MASK)

// typedef struct {
//     spinlock_t lock;
//     uint64_t*  top_lvl;
//     VEC_TYPE() mmap_ranges;
//
// } pagemap;

#endif
