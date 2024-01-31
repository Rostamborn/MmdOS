#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>

void* u_alloc(uint64_t size);

void u_free(void* addr);

#endif
