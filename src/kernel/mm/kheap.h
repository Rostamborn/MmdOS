#ifndef KHEAP_H
#define KHEAP_H

#include "vmm.h"
#include <stdint.h>

typedef struct {
    vm_arena* arena;
} k_heap_metadata;

void* k_heap_alloc(uint64_t size);

void k_heap_free(void* ptr);

#endif
