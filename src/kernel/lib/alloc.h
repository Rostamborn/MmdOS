#ifndef KERNEL_ALLOC_H
#define KERNEL_ALLOC_H

#include "../mm/kheap.h"

static inline void* kalloc(uint64_t size) { return k_alloc(size); }

static inline void kfree(void* ptr) { k_free(ptr); }

#endif
