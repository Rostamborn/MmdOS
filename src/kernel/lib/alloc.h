#ifndef KERNEL_ALLOC_H
#define KERNEL_ALLOC_H

#include "../mm/kheap.h"

#define KALLOC(type) (kalloc(sizeof(type)))

static inline void* kalloc(uint64_t size) { return k_heap_alloc(size); }

static inline void kfree(void* ptr) { k_heap_free(ptr); }

// static inline void* krealloc(void* ptr, uint64_t size) {
//     return slab_realloc(ptr, size);
// }

#endif
