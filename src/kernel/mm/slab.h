#ifndef KERNEL_SLAB_H
#define KERNEL_SLAB_H

#include <stdint.h>

void* slab_alloc(uint64_t size);

void slab_free(void* addr);

void* slab_realloc(void* addr, uint64_t new_size);

void slab_init();

#endif
