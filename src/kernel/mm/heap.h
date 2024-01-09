#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

void* heap_alloc(uint64_t size);

void heap_free(void* ptr);

#endif
