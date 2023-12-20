#include <stdint.h>

extern volatile struct limine_memmap_request memmap_req;

void pmm_init();

void* pmm_alloc(uint64_t n_pages);

void pmm_free(void* addr, uint64_t n_pages);
