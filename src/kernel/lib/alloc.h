#ifndef KERNEL_ALLOC_H
#define KERNEL_ALLOC_H

// #include "../mm/slab.h"
#include "../mm/kheap.h"

// #define KALLOC(type) (kalloc(sizeof(type)))

// static inline void* kalloc(uint64_t size) { return slab_alloc(size); }
static inline void* kalloc(uint64_t size) { return k_alloc(size); }

// static inline void kfree(void* ptr) { slab_free(ptr); }
static inline void kfree(void* ptr) { k_free(ptr); }

// static inline void* krealloc(void* ptr, uint64_t size) {
//     return slab_realloc(ptr, size);
// }

#endif
