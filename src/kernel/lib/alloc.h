#ifndef KERNEL_ALLOC_H
#define KERNEL_ALLOC_H

#include "../mm/heap.h"
#include "../mm/kheap.h"

static inline void* kalloc(uint64_t size) { return k_alloc(size); }

static inline void kfree(void* ptr) { k_free(ptr); }

// static inline void* krealloc(void* ptr, uint64_t size) {
//     return slab_realloc(ptr, size);
// }

static inline void* malloc(uint64_t size) { return u_alloc(size); }

static inline void free(void* ptr) { u_free(ptr); }

#endif
