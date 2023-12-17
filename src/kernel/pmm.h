#include <stdint.h>

void pmm_init();

void* pmm_alloc(uint64_t n_pages);

void pmm_free(void* addr, uint64_t n_pages);
