#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>

void* k_heap_alloc(uint64_t size);

void k_heap_free(void* ptr);

#endif
