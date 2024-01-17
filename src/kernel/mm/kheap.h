#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>

#define IS_POWER_2(num) (((num) & (num-1)) == 0)

#define DEFAULT_ALIGNMENT 16

typedef struct arena_t {
    struct arena_t* next;
    uintptr_t     base; // the start of allocation
    uint64_t      size; // page aligned
    uint64_t offset;
    uint64_t allocated;
} arena_t;

typedef struct {
    uint64_t arena_base;
} arena_metadata;

void* k_alloc(uint64_t size);

void k_free(void* addr);

#endif
