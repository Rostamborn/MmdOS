#include <stdint.h>

extern struct limine_memmap_request memmap_req;

extern struct limine_hhdm_request hhdm_req;

#define HHDM_OFFSET (hhdm_req.response->offset)

void pmm_init();

void* pmm_alloc(uint64_t n_pages);

void* pmm_alloc_nozero(uint64_t n_pages);

void pmm_free(void* addr, uint64_t n_pages);
